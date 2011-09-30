/*
 * Players status routines
 *
 * @(#)pstats.c	9.0	(rdk)	 7/17/84
 *
 * Super-Rogue
 * Copyright (C) 1984 Robert D. Kindelberger
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "rogue.h"
#include "rogue.ext"


/*
 * chg_hpt:
 *	Changes players hit points
 */
chg_hpt(howmany, alsomax, what)
int howmany;
bool alsomax;
char what;
{
	nochange = FALSE;
	if(alsomax)
		him->s_maxhp += howmany;
	him->s_hpt += howmany;
	if (him->s_hpt < 1) {
		msg(" ");
		death(what);
	}
}


/*
 * rchg_str:
 *	Update the players real strength 
 */
rchg_str(amt)
int amt;
{
	chg_abil(STR,amt,TRUE);
}

/*
 * chg_abil:
 *	Used to modify the hero's abilities
 */
chg_abil(what,amt,how)
int amt, what, how;
{
	if (amt == 0)
		return;
	if (how == TRUE) {			/* real (must be 1st) */
		updabil(what,amt,&pstats.s_re,TRUE);
		how = FALSE;
	}
	updabil(what,amt,&pstats.s_ef,how);	/* effective */
	updpack();
	wghtchk(FALSE);
}

/*
 * updabil:
 *	Do the actual abilities updating
 */
updabil(what, amt, pst, how)
struct real *pst;
int what, amt, how;
{
	register int *wh, *mx, *mr;
	struct real *mst, *msr;
	bool is_str = FALSE;
	int rtype;

	msr = &him->s_re;
	if (how == TRUE)				/* max real abilities */
		mst = &max_stats.s_re;
	else							/* max effective abil */
		mst = &max_stats.s_ef;
	switch (what) {
		case STR:
			is_str = TRUE;
			wh = &pst->a_str;
			mx = &mst->a_str;
			mr = &msr->a_str;
			rtype = R_ADDSTR;
		when DEX:
			wh = &pst->a_dex;
			mx = &mst->a_dex;
			mr = &msr->a_dex;
			rtype = R_DEX;
		when CON:
			wh = &pst->a_con;
			mx = &mst->a_con;
			mr = &msr->a_con;
			rtype = R_CONST;
		when WIS:
			wh = &pst->a_wis;
			mx = &mst->a_wis;
			mr = &msr->a_wis;
			rtype = R_KNOW;
		otherwise:
			return;
	}
	*wh += amt;						/* update by amt */
	if (amt < 0) {					/* if decrement */
		if (*wh < MINABIL)			/* minimum = 3 */
			*wh = MINABIL;
		if (how == FALSE) {
			if (*wh < *mr)			/* if less than real abil */
				*wh = *mr;			/* make equal to real */
		}
	}
	else {							/* increment */
		int themax;

		themax = MAXOTHER;				/* default maximum */
		if (is_str)
			themax = MAXSTR;			/* strength maximum */
		if (how != TRUE)
			themax += ringex(rtype);	/* get ring extra */
		if (*wh > themax) {				/* see if > max (if real) */
			*wh = themax;				/* max = 18  (24 if str) */
		}
		/*
		 * Check for updating the max player stats.
		 */
		if (*wh > *mx)
			*mx = *wh;
	}
}


/*
 * add_haste:
 *	add a haste to the player
 */
add_haste(potion)
bool potion;
{
	if (pl_on(ISHASTE)) {
		msg("You faint from exhaustion.");
		player.t_nocmd += rnd(8);
		player.t_flags &= ~ISHASTE;
		extinguish(nohaste);
	}
	else {
		player.t_flags |= ISHASTE;
		if (potion)
			fuse(nohaste, TRUE, roll(10,10));
		else
			fuse(nohaste, TRUE, roll(40,20));
	}
}

/*
 * getpdex:
 *	Gets players added dexterity for fighting
 */
getpdex(who, heave)
struct stats *who;
bool heave;
{
	reg int edex;

	edex = who->s_ef.a_dex;
	if (heave) {				/* an object was thrown here */
		if (edex > 18)
			return (edex - 15);
		switch(edex) {
			case 18: return 3;
			case 17: return 2;
			case 16: return 1;
			case 15:
			case 14:
			case 13:
			case 12:
			case 11:
			case 10:
			case 9:
			case 8:
			case 7:
			case 6: return 0;
			case 5: return -1;
			case 4: return -2;
			default: return -3;
		}
	}
	else {		/* object NOT thrown here (affects armor class) */
		if (edex > 18)
			return (14 - edex);
		switch(edex) {
			case 18: return -4;
			case 17: return -3;
			case 16: return -2;
			case 15: return -1;
			case 14:
			case 13:
			case 12:
			case 11:
			case 10:
			case 9:
			case 8: 
			case 7: return 0;
			case 6: return 1;
			case 5: return 2;
			case 4: return 3;
			default: return 4;
		}
	}
}

/*
 * getpwis:
 *	Get a players wisdom for fighting
 */
getpwis(who)
struct stats *who;
{
	reg int ewis;

	ewis = who->s_ef.a_wis;
	if (ewis > 18)
		return (ewis - 14);
	switch(ewis) {
		case 18: return 4;
		case 17: return 3;
		case 16: return 2;
		case 15: return 1;
		case 14:
		case 13:
		case 12:
		case 11:
		case 10:
		case 9:
		case 8: return 0;
		case 7:
		case 6: return -1;
		case 5:
		case 4: return -2;
		default: return -3;
	}
}

/*
 * getpcon:
 *	Get added hit points from players constitution
 */
getpcon(who)
struct stats *who;
{
	reg int econ;

	econ = who->s_ef.a_con;
	if (econ > 18)
		return (econ - 14);
	switch(econ) {
		case 18: return 4;
		case 17: return 3;
		case 16: return 2;
		case 15: return 1;
		case 14:
		case 13:
		case 12:
		case 11:
		case 10:
		case 9:
		case 8:
		case 7: return 0;
		case 6:
		case 5:
		case 4: return -1;
		default: return -2;
	}
}


/*
 * str_plus:
 *	compute bonus/penalties for strength on the "to hit" roll
 */
str_plus(who)
struct stats *who;
{
	reg int hitplus, str;

	hitplus = 0;
	str = who->s_ef.a_str;
	if (str > 24)			/* > 24 */
		hitplus = str - 21;
	else if (str == 24)		/* 24 */
		hitplus = 3;
	else if (str > 20)		/* 21 to 23 */
		hitplus = 2;
	else if(str >= 17)		/* 17 to 20 */
		hitplus = 1;
	else if(str > 7)		/* 8 to 16 */
		hitplus = 0;
	else if(str > 5)		/* 6 to 7 */
		hitplus = -1;
	else if(str > 3)		/* 4 to 5 */
		hitplus = -2;
	else
		hitplus = -3;		/* < 4 */
	if (who == him)			/* add pack weight if hero */
		hitplus += hitweight();
	return hitplus;
}


/*
 * add_dam:
 *	Compute additional damage done depending on strength
 */
add_dam(who)
struct stats *who;
{
	reg int exdam, str;

	exdam = 0;
	str = who->s_ef.a_str;
	if (str > 24)			/* > 24 */
		exdam = str - 18;
	else if (str == 24)		/* 24 */
		exdam = 6;
	else if (str == 23)		/* 23 */
		exdam = 5;
	else if (str > 20)		/* 21 to 22 */
		exdam = 4;
	else if (str > 18)		/* 19 to 20 */
		exdam = 3;
	else if (str == 18)		/* 18 */
		exdam = 2;
	else if (str > 15)		/* 16 to 17 */
		exdam = 1;
	else if (str > 6)		/* 7 to 14 */
		exdam = 0;
	else
		exdam = -1;			/* 3 to 6 */
	if (who == him)
		exdam += hungdam();		/* add hungry state if hero */
	return exdam;
}


/*
 * hungdam:
 *	Calculate damage depending on players hungry state
 */
hungdam()
{
	switch (hungry_state) {
		case F_OKAY:
		case F_HUNGRY:	return 0;
		when F_WEAK:	return -1;
		when F_FAINT:	return -2;
	}
}

/*
 * heal_self:
 *	Heal the hero.
 */
heal_self(factor, updmaxhp)
int factor;
bool updmaxhp;
{
	him->s_hpt += roll(him->s_lvl + getpcon(him), factor);
	if (updmaxhp)
		him->s_maxhp += 1;
	if (him->s_hpt > him->s_maxhp)
		him->s_hpt = him->s_maxhp;
	nochange = FALSE;
}
