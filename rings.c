/*
 * routines dealing specifically with rings
 *
 * @(#)rings.c	9.0	(rdk)	 7/17/84
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

/*
 * ring_on:
 *	Put on a ring
 */
ring_on()
{
	reg struct object *obj;
	reg struct linked_list *item;
	reg int ring, wh;
	char buf[LINLEN];
	bool okring;

	if (cur_ring[LEFT] != NULL && cur_ring[RIGHT] != NULL) {
		msg("Already wearing two rings.");
		after = FALSE;
		return;
	}
	/*
	 * Make certain that it is somethings that we want to wear
	 */
	if ((item = get_item("put on", RING)) == NULL)
		return;
	obj = OBJPTR(item);
	if (obj->o_type != RING) {
		msg("That won't fit on your finger.");
		return;
	}
	/*
	 * find out which hand to put it on
	 */
	if (is_current(obj))
		return;
	if (cur_ring[LEFT] == NULL && cur_ring[RIGHT] == NULL) {
		if ((ring = gethand(FALSE)) < 0)
			return;
	}
	else if (cur_ring[LEFT] == NULL)
		ring = LEFT;
	else
		ring = RIGHT;
	cur_ring[ring] = obj;
	wh = obj->o_which;
	/*
	 * okring = FALSE when:
	 * 1) ring is cursed and benefit = plus
	 * 2) ring is blessed and benefit = minus
	 */
	okring = !((obj->o_ac > 0 && o_on(obj, ISCURSED)) ||
	          (obj->o_ac < 0 && o_on(obj, ISBLESS)));
	/*
	 * Calculate the effect it has on the poor guy (if possible).
	 */
	if (okring) {
		switch (wh) {
			case R_SPEED:
				if (--obj->o_ac < 0) {
					obj->o_ac = 0;
					setoflg(obj,ISCURSED);
				}
				else {
					add_haste(FALSE);
					msg("You find yourself moving must faster.");
				}
			when R_GIANT:				/* to 24 */
				him->s_ef.a_str = MAXSTR;
			when R_ADDSTR:
				chg_abil(STR,obj->o_ac,FROMRING);
			when R_KNOW:
				chg_abil(WIS,obj->o_ac,FROMRING);
			when R_DEX:
				chg_abil(DEX,obj->o_ac,FROMRING);
			when R_CONST:
				chg_abil(CON,obj->o_ac,FROMRING);
			when R_SEEINVIS:
				player.t_flags |= CANSEE;
				light(&hero);
				mvwaddch(cw, hero.y, hero.x, PLAYER);
			when R_AGGR:
				aggravate();
			when R_HEAVY:
				updpack();			/* new pack weight */
			when R_BLIND:
				r_know[R_BLIND] = TRUE;
				player.t_flags |= ISBLIND;
				look(FALSE);
			when R_SLOW:
				player.t_flags |= ISSLOW;
			when R_SAPEM:
				fuse(sapem,TRUE,150);
			when R_LIGHT: {
				struct room *rop;

				r_know[R_LIGHT] = TRUE;
				if ((rop = player.t_room) != NULL) {
					rop->r_flags &= ~ISDARK;
					light(&hero);
					mvwaddch(cw, hero.y, hero.x, PLAYER);
				}
			}
		}
	}
	if (r_know[wh] && r_guess[wh]) {
		free(r_guess[wh]);
		r_guess[wh] = NULL;
	}
	else if(!r_know[wh] && r_guess[wh] == NULL) {
		mpos = 0;
		strcpy(buf, r_stones[wh]);
		msg(callit);
		if (get_str(buf, cw) == NORM) {
			r_guess[wh] = new(strlen(buf) + 1);
			strcpy(r_guess[wh], buf);
		}
	}
	mpos = 0;
	msg("Now wearing %s",inv_name(obj,TRUE));
	ringfood = ring_eat();
	nochange = FALSE;
}


/*
 * ring_off:
 *	Take off some ring
 */
ring_off()
{
	reg int ring;
	reg struct object *obj;
	
	if (cur_ring[LEFT] == NULL && cur_ring[RIGHT] == NULL) {
		msg("You're not wearing any rings.");
		return;
	}
	else if (cur_ring[LEFT] == NULL)
		ring = RIGHT;
	else if (cur_ring[RIGHT] == NULL)
		ring = LEFT;
	else
		if ((ring = gethand(TRUE)) < 0)
			return;
	mpos = 0;
	obj = cur_ring[ring];
	if (obj == NULL) {
		msg("Not wearing such a ring.");
		return;
	}
	if (dropcheck(obj)) {
		msg("Was wearing %s", inv_name(obj, TRUE));
		nochange = FALSE;
		ringfood = ring_eat();
	}
}


/*
 * toss_ring:
 *	Remove a ring and stop its effects
 */
toss_ring(what)
struct object *what;
{
	bool okring;

	/*
	 * okring = FALSE when:
	 * 1) ring is cursed and benefit = plus
	 * 2) ring is blessed and benefit = minus
	 */
	okring = !((what->o_ac > 0 && o_on(what, ISCURSED)) ||
	          (what->o_ac < 0 && o_on(what, ISBLESS)));

	cur_ring[what == cur_ring[LEFT] ? LEFT : RIGHT] = NULL;
	if (okring) {
		switch (what->o_which) {
			case R_SPEED:
				extinguish(nohaste);
				nohaste(FALSE);
			when R_BLIND:
				sight(FALSE);
			when R_SLOW:
				player.t_flags &= ~ISSLOW;
			when R_SAPEM:
				extinguish(sapem);
			when R_GIANT:
				him->s_ef = him->s_re;
				ringabil();
			when R_ADDSTR:
				chg_abil(STR,-what->o_ac,FALSE);
			when R_KNOW:
				chg_abil(WIS,-what->o_ac,FALSE);
			when R_DEX:
				chg_abil(DEX,-what->o_ac,FALSE);
			when R_CONST:
				chg_abil(CON,-what->o_ac,FALSE);
			when R_SEEINVIS:
				player.t_flags &= ~CANSEE;
				extinguish(unsee);
				light(&hero);
				mvwaddch(cw, hero.y, hero.x, PLAYER);
		}
	}
}


/*
 * gethand:
 *	Get a hand to wear a ring
 */
gethand(isrmv)
bool isrmv;
{
	reg int c;
	char *ptr;
	struct object *obj;

	while(1) {
		addmsg("Left or Right ring");
		if (isrmv)
			addmsg(starlist);
		addmsg("? ");
		endmsg();
		c = readchar();
		if (isupper(c))
			c = tolower(c);
		if (c == '*' && isrmv) {
			wclear(hw);
			obj = cur_ring[LEFT];
			if (obj != NULL)
				ptr = inv_name(obj, TRUE);
			else
				ptr = "none";
			wprintw(hw, "L)  %s\n\r",ptr);
			obj = cur_ring[RIGHT];
			if (obj != NULL)
				ptr = inv_name(obj, TRUE);
			else
				ptr = "none";
			wprintw(hw, "R)  %s\n\r", ptr);
			wprintw(hw, "\n\r\nWhich hand? ");
			draw(hw);
			c = readchar();
			if (isupper(c))
				c = tolower(c);
			restscr(cw);
		}
		if (c == 'l')
			return LEFT;
		else if (c == 'r')
			return RIGHT;
		else if (c == ESCAPE)
			return -1;
		mpos = 0;
		msg("L or R");
	}
}

/*
 * ring_eat:
 *	How much food do the hero's rings use up?
 */
ring_eat()
{
	reg struct object *lb;
	reg int hand, i, howmuch;
	bool addit;

	howmuch = 0;
	addit = TRUE;
	for (i = LEFT; i <= RIGHT ; i += 1) {
		lb = cur_ring[i];
		if (lb != NULL) {
			switch (lb->o_which) {
				case R_REGEN:
				case R_GIANT:
					howmuch += 2;
				when R_SPEED:
				case R_SUSTSTR:
				case R_SUSAB:
					howmuch += 1;
				when R_SEARCH:
					howmuch += (rnd(100) < 33);
				when R_DIGEST:
					switch(lb->o_ac) {
						case -3: if (rnd(100) < 25)
									howmuch += 3;
						when -2: if (rnd(100) < 50)
									howmuch += 2;
						when -1: howmuch += 1;
						when  0: howmuch -= (rnd(100) < 50);
						when  3: if (rnd(100) < 25)
									howmuch -= 3;
						when  2: if (rnd(100) < 50)
									howmuch -= 2;
						default: howmuch -= 1;
					}
				otherwise:
					addit = FALSE;
			}
			if (addit) {
				if (o_on(lb, ISBLESS))
					howmuch -= 1;
				else if (o_on(lb, ISCURSED))
					howmuch += 1;
			}
		}
	}
	return howmuch;
}


/*
 * ring_num:
 *	Print ring bonuses
 */
char *
ring_num(what)
struct object *what;
{
	static char number[5];

	number[0] = '\0';
	if (o_on(what,ISKNOW) || o_on(what,ISPOST)) {
		if (magring(what)) {	/* only rings with numbers */
			number[0] = ' ';
			strcpy(&number[1], num(what->o_ac, 0));
		}
	}
	return number;
}


/*
 * magring:
 *	Returns TRUE if a ring has a number, i.e. +2
 */
magring(what)
struct object *what;
{
	switch(what->o_which) {
		case R_SPEED:
		case R_ADDSTR:
		case R_PROTECT:
		case R_ADDHIT:
		case R_ADDDAM:
		case R_DIGEST:
		case R_CONST:
		case R_KNOW:
		case R_DEX:
			return TRUE;
		default:
			return FALSE;
	}
}


/*
 * ringabil:
 *	Compute effective abilities due to rings
 */
ringabil()
{
	reg struct object *rptr;
	reg int i;

	for(i = LEFT; i <= RIGHT; i++) {
		rptr = cur_ring[i];
		if (rptr != NULL) {
			switch(rptr->o_which) {
				case R_ADDSTR:
					chg_abil(STR,rptr->o_ac,FROMRING);
				when R_DEX: 
					chg_abil(DEX,rptr->o_ac,FROMRING);
				when R_KNOW:
					chg_abil(WIS,rptr->o_ac,FROMRING);
				when R_CONST:
					chg_abil(CON,rptr->o_ac,FROMRING);
			}
		}
	}
}


/*
 * init_ring:
 *	Initialize a ring
 */
init_ring(what,fromwiz)
struct object *what;
bool fromwiz;			/* TRUE when from wizards */
{
	reg int much;

	switch (what->o_which) {
		case R_DIGEST:		/* -3 to +3 rings */
		case R_ADDSTR:
		case R_PROTECT:
		case R_ADDHIT:
		case R_ADDDAM:
		case R_DEX:
		case R_KNOW:
		case R_CONST:
			if (fromwiz) {
				much = getbless();		/* get wizards response */
			}
			else {						/* normal users */
				if (rnd(100) < 25)
					much = -rnd(3) - 1;
				else
					much = rnd(3) + 1;
			}
			what->o_ac = much;
			if (much < 0)
				setoflg(what,ISCURSED);
		when R_SPEED:
			what->o_ac = rnd(4) + 1;
		when R_AGGR:
		case R_DELUS:
		case R_HEAVY:
		case R_BLIND:
		case R_SLOW:
		case R_SAPEM:
		case R_TELEPORT:
			what->o_ac = 0;
			setoflg(what,ISCURSED);	
		when R_GIANT:
			what->o_ac = 25;		/* lots !! of STR */
		otherwise:
			what->o_ac = 1;
	}
	what->o_type = RING;
	what->o_weight = things[TYP_RING].mi_wght;
	what->o_typname = things[TYP_RING].mi_name;
	what->o_vol = itemvol(what);
}

/*
 * ringex:
 *	Get extra gains from rings
 */
ringex(rtype)
int rtype;
{
	reg int howmuch = 0;

	if (isring(LEFT, rtype))
		howmuch += cur_ring[LEFT]->o_ac;
	if (isring(RIGHT, rtype))
		howmuch += cur_ring[RIGHT]->o_ac;
	return howmuch;
}

/*
 * iswearing:
 *	Returns TRUE when the hero is wearing a certain type of ring
 */
iswearing(ring)
int ring;
{
	return (isring(LEFT,ring) || isring(RIGHT,ring));
}

/*
 * isring:
 *	Returns TRUE if a ring is on a hand
 */
isring(hand,ring)
int hand, ring;
{
	if (cur_ring[hand] != NULL && cur_ring[hand]->o_which == ring)
		return TRUE;
	return FALSE;
}
