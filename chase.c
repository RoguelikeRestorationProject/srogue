/*
 * Code for one object to chase another
 *
 * @(#)chase.c	9.0	(rdk)	 7/17/84
 *
 * Super-Rogue
 * Copyright (C) 1984 Robert D. Kindelberger
 * All rights reserved.
 *
 * Based on "Rogue: Exploring the Dungeons of Doom"
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "rogue.h"
#include "rogue.ext"

#define FARAWAY	32767
#define RDIST(a, b)	(DISTANCE((a)->y, (a)->x, (b).y, (b).x))

struct coord ch_ret;	/* Where chasing takes you */

/*
 * runners:
 *	Make all the running monsters move.
 */
runners()
{
	reg struct thing *tp;
	reg struct linked_list *mon,*nextmon;

	for (mon = mlist; mon != NULL; mon = nextmon) {
		tp = THINGPTR(mon);
		nextmon = next(mon);
		if (off(*tp, ISHELD) && on(*tp, ISRUN)) {
			if (tp->t_nomove > 0)
				if (--tp->t_nomove > 0)
					continue;
			if (on(*tp, ISHASTE))
				if (do_chase(mon) == -1)
					continue;
			if (off(*tp, ISSLOW) || tp->t_turn)
				if (do_chase(mon) == -1)
					continue;
			tp->t_turn ^= TRUE;
		}
	}
}


/*
 * do_chase:
 *	Make one thing chase another.
 */
do_chase(mon)
struct linked_list *mon;
{
	reg struct thing *th;
	reg struct room *rer, *ree, *rxx;
	reg int mindist, i, dist;
	struct stats *st;
	bool stoprun = FALSE, ondoor = FALSE, link = FALSE;
	char runaway, dofight, wound, sch, ch;
	struct coord this;
	struct trap *trp;

	th = THINGPTR(mon);
	wound = th->t_flags & ISWOUND;
	if (wound)
		mindist = 0;
	else
		mindist = FARAWAY;
	runaway = wound;
	dofight = !runaway;
	rer = th->t_room;
	if (th->t_type == 'V') {
		if (rer != NULL && !rf_on(rer, ISDARK)) {
			/*
			 * Vampires can't stand the light
			 */
			if (cansee(th->t_pos.y, th->t_pos.x))
				msg("The vampire vaporizes into thin air !");
			killed(mon, FALSE);
			return(-1);
		}
	}
	ree = roomin(th->t_dest);	/* room of chasee */
	this = *th->t_dest;
	/*
	 * If the object of our desire is in a different
	 * room, then run to the door nearest to our goal.
	 */
	if (mvinch(th->t_pos.y, th->t_pos.x) == DOOR)
		ondoor = TRUE;
	rxx = NULL;
	if (rer != NULL || ree != NULL) {
		/*
		 * Monster not in room, hero in room. Run to closest door
		 * in hero's room if not wounded. Run away if wounded.
		 */
		if (rer == NULL && ree != NULL) {
			if (!wound)
				rxx = ree;
		}
		/*
		 * Monster in a room, hero not in room. If on a door,
		 * then use closest distance. If not on a door, then
		 * run to closest door in monsters room.
		 */
		else if (rer != NULL && ree == NULL) {
			if (!ondoor) {
				rxx = rer;
				if (wound)
					runaway = FALSE;
			}
		}
		/*
		 * Both hero and monster in a DIFFERENT room. Set flag to
		 * check for links between the monster's and hero's rooms.
		 * If no links are found, then the closest door in the
		 * monster's room is used.
		 */
		else if (rer != ree) {
			if (!wound) {
				link = TRUE;
				if (ondoor)
					rxx = ree;	/* if on door, run to heros room */
				else
					rxx = rer;	/* else to nearest door this room */
			}
		}
		/*
		 * Both hero and monster in same room. If monster is
		 * wounded, find the best door to run to.
		 */
		else if (wound) {
			struct coord *ex;
			int poss, mdtd, hdtd, ghdtd, nx, gx = 0, best;

			best = ghdtd = -FARAWAY;
			for (nx = 0; nx < ree->r_nexits; nx++) {
				ex = &ree->r_exit[nx];
				if (mvinch(ex->y, ex->x) == SECRETDOOR)
					continue;
				gx += 1;
				mdtd = abs(th->t_pos.y - ex->y) + abs(th->t_pos.x - ex->x);
				hdtd = abs(hero.y - ex->y) + abs(hero.x - ex->x);
				poss = hdtd - mdtd;				/* possible move */
				if (poss > best) {
					best = poss;
					this = *ex;
				}
				else if (poss == best && hdtd > ghdtd) {
					ghdtd = hdtd;
					best = poss;
					this = *ex;
				}
			}
			runaway = FALSE;		/* go for target */
			if (best < 1)
				dofight = TRUE;		/* fight if we must */
			mdtd = (gx <= 1 && best < 1);
			if (ondoor || mdtd) {
				this = hero;
				runaway = TRUE;
				if (!mdtd)
					dofight = FALSE;
			}
		}
		if (rxx != NULL) {
			for (i = 0; i < rxx->r_nexits; i += 1) {
				dist = RDIST(th->t_dest, rxx->r_exit[i]);
				if (link && rxx->r_ptr[i] == ree)
					dist = -1;
				if ((!wound && dist < mindist) ||
				    (wound && dist > mindist)) {
					this = rxx->r_exit[i];
					mindist = dist;
				}
			}
		}
	}
	else if (DISTANCE(hero.y, hero.x, th->t_pos.y, th->t_pos.x) <= 3)
		dofight = TRUE;
	/*
	 * this now contains what we want to run to this time
	 * so we run to it.  If we hit it we either want to
	 * fight it or stop running.
	 */
	if (chase(th, &this, runaway, dofight) == FIGHT) {
		return( attack(th) );
	}
	else if ((th->t_flags & (ISSTUCK | ISPARA)))
		return(0);				/* if paralyzed or stuck */
	if ((trp = trap_at(ch_ret.y, ch_ret.x)) != NULL) {
		ch = be_trapped(&ch_ret, th);
		if (ch == GONER || nlmove) {
			if (ch == GONER)
				remove_monster(&th->t_pos, mon);
			nlmove = FALSE;
			return((ch == GONER) ? -1 : 0);
		}
	}
	if (pl_off(ISBLIND))
		mvwaddch(cw,th->t_pos.y,th->t_pos.x,th->t_oldch);
	sch = mvwinch(cw, ch_ret.y, ch_ret.x);
	if (rer != NULL && rf_on(rer,ISDARK) && sch == FLOOR &&
	  DISTANCE(ch_ret.y,ch_ret.x,th->t_pos.y,th->t_pos.x) < 3 &&
	  pl_off(ISBLIND))
		th->t_oldch = ' ';
	else
		th->t_oldch = sch;
	if (cansee(unc(ch_ret)) && off(*th, ISINVIS))
		mvwaddch(cw, ch_ret.y, ch_ret.x, th->t_type);
	mvwaddch(mw, th->t_pos.y, th->t_pos.x, ' ');
	mvwaddch(mw, ch_ret.y, ch_ret.x, th->t_type);
	th->t_oldpos = th->t_pos;
	th->t_pos = ch_ret;
	th->t_room = roomin(&ch_ret);
	i = 5;
	if (th->t_flags & ISREGEN)
		i = 40;
	st = &th->t_stats;
	if (rnd(100) < i) {
		if (++st->s_hpt > st->s_maxhp)
			st->s_hpt = st->s_maxhp;
		if (!monhurt(th))
			th->t_flags &= ~ISWOUND;
	}
	if (stoprun && ce(th->t_pos, *(th->t_dest)))
		th->t_flags &= ~ISRUN;
	return CHASE;
}


/*
 * chase:
 *	Find the spot for the chaser to move closer to the
 *	chasee.  Returns TRUE if we want to keep on chasing
 *	later FALSE if we reach the goal.
 */
chase(tp, ee, runaway, dofight)
struct thing *tp;
struct coord *ee;
bool runaway, dofight;
{
	reg int x, y, ch;
	reg int dist, thisdist, closest;
	reg struct coord *er = &tp->t_pos;
	struct coord try, closecoord;
	int numsteps, onscare;

	/*
	 * If the thing is confused, let it move randomly.
	 */
	ch = CHASE;
	onscare = FALSE;
	if (on(*tp, ISHUH)) {
		ch_ret = *rndmove(tp);
		dist = DISTANCE(hero.y, hero.x, ch_ret.y, ch_ret.x);
		if (rnd(1000) < 5)
			tp->t_flags &= ~ISHUH;
		if (dist == 0)
			ch = FIGHT;
	}
	else {
		/*
		 * Otherwise, find the the best spot to run to
		 * in order to get to your goal.
		 */
		numsteps = 0;
		if (runaway)
			closest = 0;
		else
			closest = FARAWAY;
		ch_ret = *er;
		closecoord = tp->t_oldpos;
		for (y = er->y - 1; y <= er->y + 1; y += 1) {
			for (x = er->x - 1; x <= er->x + 1; x += 1) {
				if (!cordok(y, x))
					continue;
				try.x = x;
				try.y = y;
				if (!diag_ok(er, &try))
					continue;
				ch = winat(y, x);
				if (step_ok(ch)) {
					struct trap *trp;

					if (isatrap(ch)) {
						trp = trap_at(y, x);
						if (trp != NULL && off(*tp, ISHUH)) {
							/*
							 * Dont run over found traps unless
							 * the hero is standing on it. If confused,
							 * then he can run into them.
							 */
							if (trp->tr_flags & ISFOUND) {
								if (trp->tr_type == POOL && rnd(100) < 80)
									continue;
								else if (y != hero.y || x != hero.x)
									continue;
							}
						}
					}
					/*
					 * Check for scare monster scrolls.
					 */
					if (ch == SCROLL) {
						struct linked_list *item;

						item = find_obj(y, x);
						if (item != NULL)
							if ((OBJPTR(item))->o_which == S_SCARE) {
								if (ce(hero, try))
									onscare = TRUE;
								continue;
							}
					}
					/*
					 * Vampires will not run into a lit room.
					 */
					if (tp->t_type == 'V') {
						struct room *lr;

						lr = roomin(&try);
						if (lr != NULL && !rf_on(lr, ISDARK))
							continue;
					}
					/*
					 * This is a valid place to step
					 */
					if (y == hero.y && x == hero.x) {
						if (dofight) {
							ch_ret = try;	/* if fighting */
							return FIGHT;	/* hit hero */
						}
						else
							continue;
					}
					thisdist = DISTANCE(y, x, ee->y, ee->x);
					if (thisdist <= 0) {
						ch_ret = try;	/* got here but */
						return CHASE;	/* dont fight */
					}
					numsteps += 1;
					if ((!runaway && thisdist < closest) ||
						(runaway && thisdist > closest)) {
						/*
						 * dont count the monsters last position as
						 * the closest spot, unless running away and
						 * in the same room.
						 */
						if (!ce(try, tp->t_oldpos) || (runaway
						  && player.t_room == tp->t_room
						  && tp->t_room != NULL)) {
							closest = thisdist;
							closecoord = try;
						}
					}
				}
			}
		}
		/*
		 * If dead end, then go back from whence you came.
		 * Otherwise, pick the closest of the remaining spots.
		 */
		if (numsteps > 0)			/* move to best spot */
			ch_ret = closecoord;
		else {						/* nowhere to go */
			if (DISTANCE(tp->t_pos.y, tp->t_pos.x, hero.y, hero.x) < 2)
				if (!onscare)
					ch_ret = hero;
		}
		if (ce(hero, ch_ret))
			ch = FIGHT;
	}
	return ch;
}


/*
 * runto:
 *	Set a monster running after something
 */
runto(runner, spot)
struct coord *runner;
struct coord *spot;
{
	reg struct linked_list *item;
	reg struct thing *tp;

	if ((item = find_mons(runner->y, runner->x)) == NULL)
		return;
	tp = THINGPTR(item);
	if (tp->t_flags & ISPARA)
		return;
	tp->t_dest = spot;
	tp->t_flags |= ISRUN;
	tp->t_flags &= ~ISHELD;
}


/*
 * roomin:
 *	Find what room some coordinates are in.
 *	NULL means they aren't in any room.
 */
struct room *
roomin(cp)
struct coord *cp;
{
	reg struct room *rp;

	if (cordok(cp->y, cp->x)) {
		for (rp = rooms; rp < &rooms[MAXROOMS]; rp += 1)
			if (inroom(rp, cp))
				return rp;
	}
	return NULL;
}


/*
 * find_mons:
 *	Find the monster from his coordinates
 */
struct linked_list *
find_mons(y, x)
int y, x;
{
	reg struct linked_list *item;
	reg struct thing *th;

	for (item = mlist; item != NULL; item = next(item)) {
		th = THINGPTR(item);
		if (th->t_pos.y == y && th->t_pos.x == x)
			return item;
	}
	return NULL;
}


/*
 * diag_ok:
 *	Check to see if the move is legal if it is diagonal
 */
diag_ok(sp, ep)
struct coord *sp, *ep;
{
	if (ep->x == sp->x || ep->y == sp->y)
		return TRUE;
	if (step_ok(mvinch(ep->y,sp->x)) && step_ok(mvinch(sp->y,ep->x)))
		return TRUE;
	return FALSE;
}


/*
 * cansee:
 *	returns true if the hero can see a certain coordinate.
 */
cansee(y, x)
int y, x;
{
	reg struct room *rer;
	struct coord tp;

	if (pl_on(ISBLIND))
		return FALSE;
	/*
	 * We can only see if the hero in the same room as
	 * the coordinate and the room is lit or if it is close.
	 */
	if (DISTANCE(y, x, hero.y, hero.x) < 3)
		return TRUE;
	tp.y = y;
	tp.x = x;
	rer = roomin(&tp);
	if (rer != NULL && levtype != MAZELEV)
		if (rer == player.t_room && !rf_on(rer,ISDARK))
			return TRUE;
	return FALSE;
}
