/*
 * File with various monster functions in it
 *
 * @(#)monsters.c	9.0	(rdk)	 7/17/84
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
#include <ctype.h>
#include "rogue.ext"

/*
 * rnd_mon:
 *	Pick a monster to show up.  The lower the level,
 *	the meaner the monster.
 */
rnd_mon(wander,baddie)
bool wander;
bool baddie;		/* TRUE when from a polymorph stick */
{
	reg int i, ok, cnt;

	cnt = 0;
	if (levcount == 0)			/* if only asmodeus possible */
		return(MAXMONS);
	if (baddie) {
		while (1) {
			i = rnd(MAXMONS);					/* pick ANY monster */
			if (monsters[i].m_lev.l_lev < 0)	/* skip genocided ones */
				continue;
			return i;
		}
	}
	ok = FALSE;
	do {
		/*
		 * get a random monster from this range
		 */
		i = rnd(levcount);
		/*
		 * Only create a wandering monster if we want one
		 * (or the count is exceeded)
		 */
		if (!wander || mtlev[i]->m_lev.d_wand || ++cnt > 500)
			ok = TRUE;
	} while(!ok);
	return (midx(mtlev[i]->m_show));
}

/*
 * lev_mon:
 *	This gets all monsters possible on this level
 */
lev_mon()
{
	reg int i;
	reg struct monster *mm;

	levcount = 0;
	for (i = 0; i < MAXMONS; i++) {
		mm = &monsters[i];
		if (mm->m_lev.h_lev >= level && mm->m_lev.l_lev <= level) {
			mtlev[levcount] = mm;
			if (++levcount >= MONRANGE)
				break;
		}
	}
	if (levcount == 0)					/* if no monsters are possible */
		mtlev[0] = &monsters[MAXMONS];	/* then asmodeus 'A' */
}

/*
 * new_monster:
 *	Pick a new monster and add it to the list
 */
struct linked_list *
new_monster(type, cp, treas)
struct coord *cp;
bool treas;
char type;
{
	reg struct linked_list *item;
	reg struct thing *tp;
	reg struct monster *mp;
	reg struct stats *st;
	float killexp;		/* experience gotten for killing him */

	item = new_item(sizeof(struct thing));
	attach(mlist, item);
	tp = THINGPTR(item);
	st = &tp->t_stats;
	mp = &monsters[type];		/* point to this monsters structure */
	tp->t_type = mp->m_show;
	tp->t_indx = type;
	tp->t_pos = *cp;
	tp->t_room = roomin(cp);
	tp->t_oldch = mvwinch(cw, cp->y, cp->x);
	tp->t_nomove = 0;
	tp->t_nocmd = 0;
	mvwaddch(mw, cp->y, cp->x, tp->t_type);

	/*
	 * copy monster data
	 */
	tp->t_stats = mp->m_stats;

	/*
	 * If below amulet level, make the monsters meaner the
	 * deeper the hero goes.
	 */
	if (level > AMLEVEL)
		st->s_lvl += ((level - AMLEVEL) / 4);

	/*
	 * If monster in treasure room, then tougher.
	 */
	if (treas)
		st->s_lvl += 1;
	if (levtype == MAZELEV)
		st->s_lvl += 1;
	/*
	 * If the hero is going back up, then the monsters are more
	 * prepared for him, so tougher.
	 */
	if (goingup())
		st->s_lvl += 1;

	/*
	 * Get hit points for monster depending on his experience
	 */
	st->s_hpt = roll(st->s_lvl, 8);
	st->s_maxhp = st->s_hpt;
	/*
	 * Adjust experience point we get for killing it by the
	 *  strength of this particular monster by ~~ +- 50%
	 */
	killexp = mp->m_stats.s_exp * (0.47 + (float)st->s_hpt /
		(8 * (float)st->s_lvl));

	st->s_exp = killexp;			/* use float for accuracy */
	if(st->s_exp < 1)
		st->s_exp = 1;				/* minimum 1 experience point */
	tp->t_flags = mp->m_flags;
	/*
	 * If monster in treasure room, then MEAN
	 */
	if (treas || levtype == MAZELEV)
		tp->t_flags |= ISMEAN;
	tp->t_turn = TRUE;
	tp->t_pack = NULL;
	/*
	 * Dont wander if treas room
	 */
	if (iswearing(R_AGGR) && !treas)
		runto(cp, &hero);
	if (tp->t_type == 'M') {
		char mch;

		if (tp->t_pack != NULL)
			mch = (OBJPTR(tp->t_pack))->o_type;
		else {
			switch (rnd(level >= AMLEVEL ? 9 : 8)) {
				case 0: mch = GOLD;
				when 1: mch = POTION;
				when 2: mch = SCROLL;
				when 3: mch = STAIRS;
				when 4: mch = WEAPON;
				when 5: mch = ARMOR;
				when 6: mch = RING;
				when 7: mch = STICK;
				when 8: mch = AMULET;
			}
		}
		if (treas)
			mch = 'M';		/* no disguise in treasure room */
		tp->t_disguise = mch;
	}
	return item;
}

/*
 * wanderer:
 *	A wandering monster has awakened and is headed for the player
 */
wanderer()
{
	reg int ch;
	reg struct room *rp, *hr = player.t_room;
	reg struct linked_list *item;
	reg struct thing *tp;
	struct coord mp;

	do {
		rp = &rooms[rnd_room()];
		if (rp != hr || levtype == MAZELEV) {
			mp = *rnd_pos(rp);
			ch = mvinch(mp.y, mp.x);
		}
	} while (!step_ok(ch));
	item = new_monster(rnd_mon(TRUE,FALSE), &mp, FALSE);
	tp = THINGPTR(item);
	tp->t_flags |= ISRUN;
	tp->t_dest = &hero;
}

/*
 * wake_monster:
 *	What to do when the hero steps next to a monster
 */
struct linked_list *
wake_monster(y, x)
int y, x;
{
	reg struct thing *tp;
	reg struct linked_list *it;
	reg struct room *rp;
	reg char ch;
	bool treas = FALSE;

	if ((it = find_mons(y, x)) == NULL)
		return NULL;
	tp = THINGPTR(it);
	ch = tp->t_type;
	/*
	 * Every time he sees mean monster, it might start chasing him
	 */
	rp = player.t_room;
	if (rp != NULL && rf_on(rp,ISTREAS)) {
		tp->t_flags &= ~ISHELD;
		treas = TRUE;
	}
	if (treas || (rnd(100) > 33 && on(*tp,ISMEAN) && off(*tp,ISHELD) &&
	  !iswearing(R_STEALTH))) {
		tp->t_dest = &hero;
		tp->t_flags |= ISRUN;
	}
	if (ch == 'U' && pl_off(ISBLIND)) {
		if ((rp != NULL && !rf_on(rp,ISDARK) && levtype != MAZELEV)
		  || DISTANCE(y, x, hero.y, hero.x) < 3) {
			if (off(*tp,ISFOUND) && !save(VS_PETRIFICATION)
			  && !iswearing(R_SUSAB) && pl_off(ISINVINC)) {
				msg("The umber hulk's gaze has confused you.");
				if (pl_on(ISHUH))
					lengthen(unconfuse,rnd(20)+HUHDURATION);
				else
					fuse(unconfuse,TRUE,rnd(20)+HUHDURATION);
				player.t_flags |= ISHUH;
			}
			tp->t_flags |= ISFOUND;
		}
	}
	/*
	 * Hide invisible monsters
	 */
	if ((tp->t_flags & ISINVIS) && pl_off(CANSEE))
		ch = mvinch(y, x);
	/*
	 * Let greedy ones guard gold
	 */
	if (on(*tp, ISGREED) && off(*tp, ISRUN)) {
		if (rp != NULL && rp->r_goldval) {
			tp->t_dest = &rp->r_gold;
			tp->t_flags |= ISRUN;
		}
	}
	return it;
}

/*
 * genocide:
 *	Eradicate a monster forevermore
 */
genocide()
{
	reg struct linked_list *ip, *nip;
	reg struct thing *mp;
	struct monster *mm;
	reg int i, ii, c;

	if (levcount == 0) {
		mpos = 0;
		msg("You cannot genocide Asmodeus !!");
		return;
	}
tryagain:
	i = TRUE;		/* assume an error now */
	while (i) {
		msg("Which monster (remember UPPER & lower case)?");
		c = readchar();		/* get a char */
		if (c == ESCAPE) {	/* he can abort (the fool) */
			msg("");
			return;
		}
		if (isalpha(c))		/* valid char here */
			i = FALSE;		/* exit the loop */
		else {				/* he didn't type a letter */
			mpos = 0;
			msg("Please specify a letter between 'A' and 'z'");
		}
	}
	i = midx(c);						/* get index to monster */
	mm = &monsters[i];
	if (mm->m_lev.l_lev < 0) {
		mpos = 0;
		msg("You have already eliminated the %s.",mm->m_name);
		goto tryagain;
	}
	for (ip = mlist; ip != NULL; ip = nip) {
		mp = THINGPTR(ip);
		nip = next(ip);
		if (mp->t_type == c)
			remove_monster(&mp->t_pos, ip);
	}
	mm->m_lev.l_lev = -1;				/* get rid of it */
	mm->m_lev.h_lev = -1;
	lev_mon();							/* redo monster list */
	mpos = 0;
	msg("You have wiped out the %s.",mm->m_name);
}

/*
 * unhold:
 *	Release the player from being held
 */
unhold(whichmon)
char whichmon;
{
	switch (whichmon) {
		case 'F':
			fung_hit = 0;
			strcpy(monsters[midx('F')].m_stats.s_dmg, "000d0");
		case 'd':
			player.t_flags &= ~ISHELD;
	}
}

/*
 * midx:
 *	This returns an index to 'whichmon'
 */
midx(whichmon)
char whichmon;
{
	if (isupper(whichmon))
		return(whichmon - 'A');			/* 0 to 25 for uppercase */
	else if (islower(whichmon))
		return(whichmon - 'a' + 26);	/* 26 to 51 for lowercase */
	else
		return(MAXMONS);				/* 52 for Asmodeus */
}

/*
 * monhurt:
 *	See when monster should run or fight. Return
 *	TRUE if hit points less than acceptable.
 */
monhurt(th)
struct thing *th;
{
	reg int ewis, crithp, f1, f2;
	reg struct stats *st;

	st = &th->t_stats;
	ewis = st->s_ef.a_wis;
	if (ewis <= MONWIS)				/* stupid monsters dont know */
		return FALSE;
	f1 = st->s_maxhp / 4;			/* base hpt for being hurt */
	f2 = (ewis - MONWIS) * 5 / 3;	/* bonus for smart monsters */
	if (th->t_flags & ISWOUND)		/* if recovering from being */
		f1 *= 2;					/* wounded, then double the base */
	crithp = f1 + f2;				/* get critical hpt for hurt */
	if (crithp > st->s_maxhp)		/* only up to max hpt */
		crithp = st->s_maxhp;
	if (st->s_hpt < crithp)			/* if < critical, then still hurt */
		return TRUE;
	return FALSE;
}
