/*
 * Hero movement commands
 *
 * @(#)move.c	9.0	(rdk)	 7/17/84
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

#include <ctype.h>
#include "rogue.h"
#include "rogue.ext"

/*
 * Used to hold the new hero position
 */

struct coord nh;

/*
 * do_run:
 *	Start the hero running
 */

do_run(ch)
char ch;
{
	running = TRUE;
	after = FALSE;
	runch = ch;
}

/*
 * do_move:
 *	Check to see that a move is legal.  If it is handle the
 *	consequences (fighting, picking up, etc.)
 */

do_move(dy, dx)
int dy, dx;
{
	reg int ch;
	reg struct room *rp;

	firstmove = FALSE;
	curprice = -1;
	inpool = FALSE;

	if (player.t_nomove > 0) {
		player.t_nomove -= 1;
		msg("You are still stuck in the bear trap.");
		return;
	}
	/*
	 * Do a confused move (maybe)
	 */
	if ((rnd(100) < 80 && pl_on(ISHUH)) ||
	  (iswearing(R_DELUS) && rnd(100) < 25))
		nh = *rndmove(&player);
	else {
		nh.y = hero.y + dy;
		nh.x = hero.x + dx;
	}
	/*
	 * Check if he tried to move off the screen or make
	 *  an illegal diagonal move, and stop him if he did.
	 */
	if (!cordok(nh.y, nh.x) ||
	  (pl_off(ISETHER) && !diag_ok(&hero, &nh))) {
		after = running = FALSE;
		return;
	}
	if (running) {
		ch = winat(nh.y, nh.x);
		if (dead_end(ch)) {
			reg int gox, goy, apsg, whichway;

			gox = goy = apsg = 0;
			if (dy == 0) {
				ch = show(hero.y+1,hero.x);
				if (ch == PASSAGE) {
					apsg += 1;
					goy = 1;
				}
				ch = show(hero.y-1,hero.x);
				if (ch == PASSAGE) {
					apsg += 1;
					goy = -1;
				}
			}
			else if (dx == 0) {
				ch = show(hero.y,hero.x+1);
				if (ch == PASSAGE) {
					gox = 1;
					apsg += 1;
				}
				ch = show(hero.y,hero.x-1);
				if (ch == PASSAGE) {
					gox = -1;
					apsg += 1;
				}
			}
			if (apsg != 1) {
				running = after = FALSE;
				return;
			}
			else {			/* can still run here */
				nh.y = hero.y + goy;
				nh.x = hero.x + gox;
				whichway = (goy + 1) * 3 + gox + 1;
				switch(whichway) {
					case 0: runch = 'y';
					when 1: runch = 'k';
					when 2: runch = 'u';
					when 3: runch = 'h';
					when 4: runch = '.';	/* shouldn't do */
					when 5: runch = 'l';
					when 6: runch = 'b';
					when 7: runch = 'j';
					when 8: runch = 'n';
				}
			}
		}
	}
	if (running && ce(hero, nh))
		after = running = FALSE;
	ch = winat(nh.y, nh.x);
	if (pl_on(ISHELD) && ch != 'F' && ch != 'd') {
		msg("You are being held.");
		return;
	}
	if (pl_off(ISETHER)) {
		if (isatrap(ch)) {
			ch = be_trapped(&nh, &player);
			if (nlmove) {
				nlmove = FALSE;
				return;
			}
			else if (ch == POOL)
				inpool = TRUE;
		}
	 	else if (dead_end(ch)) {
			after = running = FALSE;
			return;
		}
		else {
			switch(ch) {
				case GOLD:	case POTION:	case SCROLL:
				case FOOD:	case WEAPON:	case ARMOR:
				case RING:	case AMULET:	case STICK:
					running = FALSE;
					take = ch;
				default:
					if (illeg_ch(ch)) {
						running = FALSE;
						mvaddch(nh.y, nh.x, FLOOR);
						teleport(rndspot, &player);
						light(&nh);
						msg("The spatial warp disappears !");
						return;
					}
			}
		}
	}
	rp = roomin(&nh);
	if (ch == DOOR) {		/* just stepped on a door */
		running = FALSE;
		if (rp != NULL && rf_on(rp, ISTREAS)) {
			struct linked_list *item;
			struct thing *tp;

			for (item = mlist; item != NULL; item = next(item)) {
				tp = THINGPTR(item);
				if (tp->t_room == rp)
					runto(&tp->t_pos, &hero);
			}
		}
	}
	else if (ch == STAIRS && pl_off(ISETHER))
		running = FALSE;
	else if (isalpha(ch) && pl_off(ISETHER)) {
		running = FALSE;
		fight(&nh, cur_weapon, FALSE);
		return;
	}
	if (rp == NULL && player.t_room != NULL)
		light(&hero);		/* exiting a room */
	else if (rp != NULL && player.t_room == NULL)
		light(&nh);			/* just entering a room */
	if (pl_on(ISBLIND))
		ch = ' ';
	else
		ch = player.t_oldch;
	mvwaddch(cw, hero.y, hero.x, ch);
	mvwaddch(cw, nh.y, nh.x, PLAYER);
	hero = nh;
	player.t_room = rp;
	player.t_oldch = mvinch(hero.y, hero.x);
}

/*
 * Called to illuminate a room.
 * If it is dark, remove anything that might move.
 */
light(cp)
struct coord *cp;
{
	reg struct room *rp;
	reg int j, k, x, y;
	reg char ch, rch;
	reg struct linked_list *item;

	rp = roomin(cp);
	if (rp == NULL)
		return;
	if (pl_on(ISBLIND)) {
		for (j = 0; j < rp->r_max.y; j += 1) {
			for (k = 0; k < rp->r_max.x; k += 1) {
				y = rp->r_pos.y + j;
				x = rp->r_pos.x + k;
				mvwaddch(cw, y, x, ' ');
			}
		}
		look(FALSE);
		return;
	}
	if (iswearing(R_LIGHT))
		rp->r_flags &= ~ISDARK;
	for (j = 0; j < rp->r_max.y; j += 1) {
		for (k = 0; k < rp->r_max.x; k += 1) {
			y = rp->r_pos.y + j;
			x = rp->r_pos.x + k;
			if (levtype == MAZELEV && !cansee(y, x))
				continue;
			ch = show(y, x);
			wmove(cw, y, x);
			/*
			 * Figure out how to display a secret door
			 */
			if (ch == SECRETDOOR) {
				if (j == 0 || j == rp->r_max.y - 1)
					ch = '-';
				else
					ch = '|';
			}
			if (isalpha(ch)) {
				struct thing *mit;

				item = wake_monster(y, x);
				if (item == NULL) {
					ch = FLOOR;
					mvaddch(y, x, ch);
				}
				else {
					mit = THINGPTR(item);
					if (mit->t_oldch == ' ')
						if (!rf_on(rp,ISDARK))
							mit->t_oldch = mvinch(y, x);
					if (levtype == MAZELEV)
						ch = mvinch(y, x);
				}
			}
			if (rf_on(rp,ISDARK)) {
				rch = mvwinch(cw, y, x);
				if (isatrap(rch)) {
					ch = rch;			/* if its a trap */
				}
				else {					/* try other things */
					switch (rch) {
						case DOOR:	case STAIRS:	case '|':
						case '-':
							ch = rch;
						otherwise:
							ch = ' ';
					}
				}
			}
			mvwaddch(cw, y, x, ch);
		}
	}
}

/*
 * show:
 *	returns what a certain thing will display as to the un-initiated
 */
show(y, x)
int y, x;
{
	reg char ch = winat(y, x);
	reg struct linked_list *it;
	reg struct thing *tp;
	reg struct trap *ta;

	if (isatrap(ch)) {
		if ((ta = trap_at(y, x)) == NULL)
			return FLOOR;
		if (iswearing(R_FTRAPS))
			ta->tr_flags |= ISFOUND;
		return ((ta->tr_flags & ISFOUND) ? ta->tr_type : FLOOR);
	}
	if (ch == SECRETDOOR && iswearing(R_FTRAPS)) {
		mvaddch(y,x,DOOR);
		return DOOR;
	}
	if ((it = find_mons(y, x)) != NULL) {	/* maybe a monster */
		tp = THINGPTR(it);
		if (ch == 'M' || (tp->t_flags & ISINVIS)) {
			if (ch == 'M')
				ch = tp->t_disguise;
			else if (pl_off(CANSEE)) {
				if (ch == 's')
					ch = ' ';		/* shadows show as a blank */
				else
					ch = mvinch(y, x);	/* hide invisibles */
			}
		}
	}
	return ch;
}

/*
 * be_trapped:
 *	Hero or monster stepped on a trap.
 */
be_trapped(tc, th)
struct thing *th;
struct coord *tc;
{
	reg struct trap *trp;
	reg int ch, ishero;
	struct linked_list *mon;
	char stuckee[35], seeit, sayso;

	if ((trp = trap_at(tc->y, tc->x)) == NULL)
		return;
	ishero = (th == &player);
	if (ishero) {
		strcpy(stuckee, "You");
		count = running = FALSE;
	}
	else {
		sprintf(stuckee, "The %s", monsters[th->t_indx].m_name);
	}
	seeit = cansee(tc->y, tc->x);
	if (seeit)
		mvwaddch(cw, tc->y, tc->x, trp->tr_type);
	trp->tr_flags |= ISFOUND;
	sayso = TRUE;
	switch (ch = trp->tr_type) {
		case POST:
			if (ishero) {
				nlmove = TRUE;
				new_level(POSTLEV);
			}
			else
				goto goner;
		when MAZETRAP:
			if (ishero) {
				nlmove = TRUE;
				level += 1;
				new_level(MAZELEV);
				msg("You are surrounded by twisty passages!");
			}
			else
				goto goner;
		when TELTRAP:
			nlmove = TRUE;
			teleport(trp->tr_goto, th);
		when TRAPDOOR:
			if (ishero) {
				level += 1;
				new_level(NORMLEV);
			}
			else {		/* monsters get lost */
goner:
				ch = GONER;
			}
			nlmove = TRUE;
			if (seeit && sayso)
				msg("%s fell into a trap!", stuckee);
		when BEARTRAP:
			th->t_nomove += BEARTIME;
			if (seeit) {
				strcat(stuckee, (ishero ? " are" : " is"));
				msg("%s caught in a bear trap.", stuckee);
			}
		when SLEEPTRAP:
			if (ishero && pl_on(ISINVINC))
				msg("You feel momentarily dizzy.");
			else {
				if (ishero)
					th->t_nocmd += SLEEPTIME;
				else
					th->t_nomove += SLEEPTIME;
				if (seeit)
					msg("%s fall%s asleep in a strange white mist.",
					  stuckee, (ishero ? "":"s"));
			}
		when ARROWTRAP: {
			int resist, ac;
			struct stats *it;

			stuckee[0] = tolower(stuckee[0]);
			it = &th->t_stats;
			if (ishero && cur_armor != NULL)
				ac = cur_armor->o_ac;
			else
				ac = it->s_arm;
			resist = ac + getpdex(it, FALSE);
			if (ishero && pl_on(ISINVINC))
				resist = -100;		/* invincible is impossible to hit */
			if (swing(3 + (level / 4), resist, 1)) {
				if (seeit)
					msg("%sAn arrow shot %s.", (ishero ? "Oh no! " : ""),
					  stuckee);
				if (ishero)
					chg_hpt(-roll(1,6),FALSE,K_ARROW);
				else {
					it->s_hpt -= roll(1,6);
					if (it->s_hpt < 1) {
						sayso = FALSE;
						goto goner;
					}
				}
			}
			else {
				struct linked_list *item;
				struct object *arrow;

				if (seeit)
					msg("An arrow shoots past %s.", stuckee);
				item = new_thing(FALSE, WEAPON, ARROW);
				arrow = OBJPTR(item);
				arrow->o_hplus = 3;
				arrow->o_dplus = rnd(2);
				arrow->o_count = 1;
		 		arrow->o_pos = th->t_pos;
				fall(item, FALSE);
			}
		}
		when DARTTRAP: {
			int resist, ac;
			struct stats *it;

			stuckee[0] = tolower(stuckee[0]);
			it = &th->t_stats;
			if (ishero && cur_armor != NULL)
				ac = cur_armor->o_ac;
			else
				ac = it->s_arm;
			resist = ac + getpdex(it, FALSE);
			if (ishero && pl_on(ISINVINC))
				resist = -100;		/* invincible is impossible to hit */
			if (swing(3 + (level / 4), resist, 0)) {
				if (seeit)
					msg("A small dart just hit %s.", stuckee);
				if (ishero) {
					if (!save(VS_POISON))
						chg_abil(CON,-1,TRUE);
					if (!iswearing(R_SUSTSTR))
						chg_abil(STR,-1,TRUE);
					chg_hpt(-roll(1, 4),FALSE,K_DART);
				}
				else {
					if (!save_throw(VS_POISON, th))
						it->s_ef.a_str -= 1;
					it->s_hpt -= roll(1, 4);
					if (it->s_hpt < 1) {
						sayso = FALSE;
						goto goner;
					}
				}
			}
			else if (seeit)
				msg("A small dart whizzes by %s.", stuckee);
		}
	    when POOL:
			if (!ishero && rnd(100) < 10) {
				if (seeit)
					msg("The %s drowns !!", stuckee);
				goto goner;
			}
			if ((trp->tr_flags & ISGONE) && rnd(100) < 10) {
				nlmove = TRUE;
				if (rnd(100) < 15)
					teleport(rndspot);	   /* teleport away */
				else if(rnd(100) < 15 && level > 2) {
					level -= rnd(2) + 1;
					new_level(NORMLEV);
					msg("You here a faint groan from below.");
				}
				else if(rnd(100) < 40) {
					level += rnd(4);
					new_level(NORMLEV);
					msg("You find yourself in strange surroundings.");
				}
				else if(rnd(100) < 6 && pl_off(ISINVINC)) {
					msg("Oh no!!! You drown in the pool!!! --More--");
					wait_for(cw, ' ');
					death(K_POOL);
				}
				else
					 nlmove = FALSE;
		}
	}
	flushinp();		/* flush typeahead */
	return ch;
}

/*
 * dip_it:
 *	Dip an object into a magic pool
 */
dip_it()
{
	reg struct linked_list *what;
	reg struct object *ob;
	reg struct trap *tp;
	reg int wh;

	tp = trap_at(hero.y,hero.x);
	if (tp == NULL || inpool == FALSE || (tp->tr_flags & ISGONE))
		return;

	if ((what = get_item("dip",0)) == NULL)
		return;
	ob = OBJPTR(what);
	mpos = 0;
	/*
	 * If hero is trying to dip an object OTHER than his
	 * current weapon, make sure that he could drop his
	 * current weapon
	 */
	if (ob != cur_weapon) {
		if (cur_weapon != NULL && o_on(cur_weapon, ISCURSED)) {
			msg("You are unable to release your weapon.");
			after = FALSE;
			return;
		}
	}
	if (ob == cur_armor) {
		msg("You have to take off your armor before you can dip it.");
		after = FALSE;
		return;
	}
	else if (ob == cur_ring[LEFT] || ob == cur_ring[RIGHT]) {
		msg("You have to take that ring off before you can dip it.");
		after = FALSE;
		return;
	}
	wh = ob->o_which;
	tp->tr_flags |= ISGONE;
	if (ob != NULL && o_off(ob,ISPROT)) {
		setoflg(ob,ISKNOW);
		switch(ob->o_type) {
		case WEAPON:
			if(rnd(100) < 20) {		/* enchant weapon here */
				if (o_off(ob,ISCURSED)) {
					ob->o_hplus += 1;
					ob->o_dplus += 1;
				}
				else {		/* weapon was prev cursed here */
					ob->o_hplus = rnd(2);
					ob->o_dplus = rnd(2);
				}
				resoflg(ob,ISCURSED);
			}
			else if(rnd(100) < 10) {	/* curse weapon here */
				if (o_off(ob,ISCURSED)) {
					ob->o_hplus = -(rnd(2)+1);
					ob->o_dplus = -(rnd(2)+1);
				}
				else {			/* if already cursed */
					ob->o_hplus--;
					ob->o_dplus--;
				}
				setoflg(ob,ISCURSED);
			}			
			msg("The %s glows for a moment.",w_magic[wh].mi_name);
		when ARMOR:
			if (rnd(100) < 30) {			/* enchant armor */
				if(o_off(ob,ISCURSED))
					ob->o_ac -= rnd(2) + 1;
				else
					ob->o_ac = -rnd(3)+ armors[wh].a_class;
				resoflg(ob,ISCURSED);
			}
			else if(rnd(100) < 15){			/* curse armor */
				if (o_off(ob,ISCURSED))
					ob->o_ac = rnd(3)+ armors[wh].a_class;
				else
					ob->o_ac += rnd(2) + 1;
				setoflg(ob,ISCURSED);
			}
			msg("The %s glows for a moment.",a_magic[wh].mi_name);
		when STICK: {
			int i;
			struct rod *rd;

			i = rnd(8) + 1;
			if(rnd(100) < 25)		/* add charges */
				ob->o_charges += i;
			else if(rnd(100) < 10) {	/* remove charges */
				if ((ob->o_charges -= i) < 0)
					ob->o_charges = 0;
			}
			ws_know[wh] = TRUE;
			rd = &ws_stuff[wh];
			msg("The %s %s glows for a moment.",rd->ws_made,rd->ws_type);
		}
		when SCROLL:
			s_know[wh] = TRUE;
			msg("The '%s' scroll unfurls.",s_names[wh]);
		when POTION:
			p_know[wh] = TRUE;
			msg("The %s potion bubbles for a moment.",p_colors[wh]);
		when RING:
			r_know[wh] = TRUE;
			if (magring(ob)) {
				if(rnd(100) < 25) {	 		/* enchant ring */
					if (o_off(ob,ISCURSED))
						ob->o_ac += rnd(2) + 1;
					else
						ob->o_ac = rnd(2) + 1;
					resoflg(ob,ISCURSED);
				}
				else if(rnd(100) < 10) {	 /* curse ring */
					if (o_off(ob,ISCURSED))
						ob->o_ac = -(rnd(2) + 1);
					else
						ob->o_ac -= (rnd(2) + 1);
					setoflg(ob,ISCURSED);
				}
			}
			msg("The %s ring vibrates for a moment.",r_stones[wh]);
		otherwise:
			msg("The pool bubbles for a moment.");
		}
	}
	cur_weapon = ob;	/* hero has to weild item to dip it */
}


/*
 * trap_at:
 *	Find the trap at (y,x) on screen.
 */
struct trap *
trap_at(y, x)
int y, x;
{
	reg struct trap *tp, *ep;

	ep = &traps[ntraps];
	for (tp = traps; tp < ep; tp += 1)
		if (tp->tr_pos.y == y && tp->tr_pos.x == x)
			break;
	if (tp >= ep)
		tp = NULL;
	return tp;
}

/*
 * rndmove:
 *	move in a random direction if the monster/person is confused
 */
struct coord *
rndmove(who)
struct thing *who;
{
	reg int x, y, ex, ey, ch;
	int nopen = 0;
	struct linked_list *item;
	static struct coord ret;  /* what we will be returning */
	static struct coord dest;

	ret = who->t_pos;
	/*
	 * Now go through the spaces surrounding the player and
	 * set that place in the array to true if the space can be
	 * moved into
	 */
	ey = ret.y + 1;
	ex = ret.x + 1;
	for (y = who->t_pos.y - 1; y <= ey; y += 1) {
		for (x = who->t_pos.x - 1; x <= ex; x += 1) {
			if (!cordok(y, x))
				continue;
			ch = winat(y, x);
			if (step_ok(ch)) {
				dest.y = y;
				dest.x = x;
				if (!diag_ok(&who->t_pos, &dest))
					continue;
				if (ch == SCROLL && who != &player) {
					/*
					 * check for scare monster scrolls
					 */
					item = find_obj(y, x);
					if (item != NULL && (OBJPTR(item))->o_which == S_SCARE)
						continue;
				}
				if (rnd(++nopen) == 0)
					ret = dest;
			}
		}
	}
	return &ret;
}

/*
 * isatrap:
 *	Returns TRUE if this character is some kind of trap
 */
isatrap(ch)
char ch;
{
	switch(ch) {
		case POST:
		case DARTTRAP:
		case POOL:
		case TELTRAP:
		case TRAPDOOR:
		case ARROWTRAP:
		case SLEEPTRAP:
		case BEARTRAP:
		case MAZETRAP:
			return TRUE;
		default:
			return FALSE;
	}
}
