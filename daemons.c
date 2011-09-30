/*
 * All the daemon and fuse functions are in here
 *
 * @(#)daemons.c	9.0	(rdk)	 7/17/84
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

int between = 0;

/*
 * doctor:
 *	A healing daemon that restores hit points after rest
 */
doctor(fromfuse)
int fromfuse;
{
	reg int *thp, lv, ohp, ccon;

	lv = him->s_lvl;
	thp = &him->s_hpt;
	ohp = *thp;
	quiet += 1;

	ccon = him->s_ef.a_con;
	if (ccon > 16 && !isfight)
		*thp += rnd(ccon - 15);
	if (lv < 8) {
		if (quiet > 20 - lv * 2)
			*thp += 1;
	}
	else {
		if (quiet >= 3)
			*thp += rnd(lv - 7) + 1;
	}
	if (isring(LEFT, R_REGEN))
		*thp += 1;
	if (isring(RIGHT, R_REGEN))
		*thp += 1;
	if (pl_on(ISREGEN))
		*thp += 1;
	if (ohp != *thp) {
		nochange = FALSE;
		if (*thp > him->s_maxhp)
			*thp = him->s_maxhp;
		quiet = 0;
	}
}


/*
 * Swander:
 *	Called when it is time to start rolling for wandering monsters
 */
swander(fromfuse)
int fromfuse;
{
	daemon(rollwand, TRUE, BEFORE);
}


/*
 * rollwand:
 *	Called to roll to see if a wandering monster starts up
 */
rollwand(fromfuse)
int fromfuse;
{

	if (++between >= 4) {
		if (roll(1, 6) == 4) {
			if (levtype != POSTLEV)		/* no monsters for posts */
				wanderer();
			extinguish(rollwand);
			fuse(swander, TRUE, WANDERTIME);
		}
		between = 0;
	}
}


/*
 * unconfuse:
 *	Release the poor player from his confusion
 */
unconfuse(fromfuse)
int fromfuse;
{
	if (pl_on(ISHUH))
		msg("You feel less confused now.");
	player.t_flags &= ~ISHUH;
}

/*
 * unsee:
 *	He lost his see invisible power
 */
unsee(fromfuse)
int fromfuse;
{
	player.t_flags &= ~CANSEE;
}

/*
 * sight:
 *	He gets his sight back
 */
sight(fromfuse)
int fromfuse;
{
	if (pl_on(ISBLIND))
		msg("The veil of darkness lifts.");
	player.t_flags &= ~ISBLIND;
	light(&hero);
}

/*
 * nohaste:
 *	End the hasting
 */
nohaste(fromfuse)
int fromfuse;
{
	if (pl_on(ISHASTE))
		msg("You feel yourself slowing down.");
	player.t_flags &= ~ISHASTE;
}


/*
 * stomach:
 *	Digest the hero's food
 */
stomach(fromfuse)
int fromfuse;
{
	reg int oldfood, old_hunger;

	old_hunger = hungry_state;
	if (food_left <= 0) {		 /* the hero is fainting */
		if (--food_left == -150) {
			msg("Your stomach writhes with hunger pains.");
		}
		else if (food_left < -350) {
			msg("You starve to death !!");
			msg(" ");
			death(K_STARVE);
		}
		if (player.t_nocmd > 0 || rnd(100) > 20)
			return;
		player.t_nocmd = rnd(8)+4;
		msg("You faint.");
		running = FALSE;
		count = 0;
		hungry_state = F_FAINT;
	}
	else {
		oldfood = food_left;
		food_left -= ringfood + foodlev - amulet;
		if (player.t_nocmd > 0)		/* wait till he can move */
			return;
		if (food_left < WEAKTIME && oldfood >= WEAKTIME) {
			msg("You are starting to feel weak.");
			hungry_state = F_WEAK;
		}
		else if(food_left < HUNGTIME && oldfood >= HUNGTIME) {
			msg("Getting hungry.");
			hungry_state = F_HUNGRY;
		}
	}
	if (old_hunger != hungry_state)
	    updpack();				/* new pack weight */
	wghtchk(FALSE);
}

/*
 * noteth:
 *	Hero is no longer etherereal
 */
noteth(fromfuse)
int fromfuse;
{
	int ch;

	if (pl_on(ISETHER)) {
		msg("You begin to feel more corporeal.");
		ch = player.t_oldch;
		if (dead_end(ch)) {
			msg("You materialize in %s.",identify(ch));
			msg(" ");
			death(K_STONE);	/* can't materialize in walls */
		}
	}
	player.t_flags &= ~ISETHER;
}

/*
 * sapem:
 *	Sap the hero's life away
 */
sapem(fromfuse)
int fromfuse;
{
	chg_abil(rnd(4) + 1, -1, TRUE);
	fuse(sapem, TRUE, 150);
	nochange = FALSE;
}

/*
 * notslow:
 *	Restore the hero's normal speed
 */
notslow(fromfuse)
int fromfuse;
{
	if (pl_on(ISSLOW))
		msg("You no longer feel hindered.");
	player.t_flags &= ~ISSLOW;
}

/*
 * notregen:
 *	Hero is no longer regenerative
 */
notregen(fromfuse)
int fromfuse;
{
	if (pl_on(ISREGEN))
		msg("You no longer feel bolstered.");
	player.t_flags &= ~ISREGEN;
}

/*
 * notinvinc:
 *	Hero not invincible any more
 */
notinvinc(fromfuse)
int fromfuse;
{
	if (pl_on(ISINVINC))
		msg("You no longer feel invincible.");
	player.t_flags &= ~ISINVINC;
}
