/*
 * Stuff to do with encumberence
 *
 * @(#)encumb.c		9.0	(rdk)	 7/17/84
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
 * updpack:
 *	Update his pack weight and adjust fooduse accordingly
 */
updpack()
{
	reg int topcarry, curcarry;

	him->s_carry = totalenc();			/* get total encumb */
	curcarry = packweight();			/* get pack weight */
	topcarry = him->s_carry / 5;		/* 20% of total carry */
	if (curcarry > 4 * topcarry) {
		if (rnd(100) < 80)
			foodlev = 3;				/* > 80% of pack */
	}
	else if (curcarry > 3 * topcarry) {
		if (rnd(100) < 60)
			foodlev = 2;				/* > 60% of pack */
	}
	else
		foodlev = 1;					/* <= 60% of pack */
	him->s_pack = curcarry;				/* update pack weight */
	packvol = pack_vol();				/* update pack volume */
	nochange = FALSE;					/* also change display */
}


/*
 * packweight:
 *	Get the total weight of the hero's pack
 */
packweight()
{
	reg struct object *obj;
	reg struct linked_list *pc;
	reg int weight, i;

	weight = 0;
	for (pc = pack ; pc != NULL ; pc = next(pc)) {
		obj = OBJPTR(pc);
		weight += itemweight(obj) * obj->o_count;
	}
	if (weight < 0)		/* in case of amulet */
		 weight = 0;
	for (i = LEFT; i <= RIGHT; i += 1) {
		obj = cur_ring[i];
		if (obj != NULL) {
			if (obj->o_type == R_HEAVY && o_off(obj, ISBLESS))
				weight += weight / 4;
		}
	}
	return weight;
}


/*
 * itemweight:
 *	Get the weight of an object
 */
itemweight(wh)
struct object *wh;
{
	reg int weight;

	weight = wh->o_weight;		/* get base weight */
	switch (wh->o_type) {
		case ARMOR:
			if ((armors[wh->o_which].a_class - wh->o_ac) > 0)
				weight /= 2;
		when WEAPON:
			if ((wh->o_hplus + wh->o_dplus) > 0)
				weight /= 2;
	}
	if (o_on(wh,ISCURSED))
		weight += weight / 5;	/* 20% more for cursed */
	if (o_on(wh, ISBLESS))
		weight -= weight / 5;	/* 20% less for blessed */
	return weight;
}

/*
 * pack_vol:
 *	Get the total volume of the hero's pack
 */
pack_vol()
{
	reg struct object *obj;
	reg struct linked_list *pc;
	reg int volume;

	volume = 0;
	for (pc = pack ; pc != NULL ; pc = next(pc)) {
		obj = OBJPTR(pc);
		volume += itemvol(obj);
	}
	return volume;
}

/*
 * itemvol:
 *	Get the volume of an object
 */
itemvol(wh)
struct object *wh;
{
	reg int volume, what, extra;

	extra = 0;
	what = getindex(wh->o_type);
	switch (wh->o_type) {
		case ARMOR:		extra = armors[wh->o_which].a_vol;
		when WEAPON:	extra = weaps[wh->o_which].w_vol;
		when STICK:		if (strcmp(ws_stuff[wh->o_which].ws_type,"staff") == 0)
							extra = V_WS_STAFF;
						else
							extra = V_WS_WAND;
	}
	volume = thnginfo[what].mf_vol + extra;
	volume *= wh->o_count;
	return volume;
}

/*
 * playenc:
 *	Get hero's carrying ability above norm
 */
playenc()
{
	reg estr = him->s_ef.a_str;
	if (estr >= 24)
		return 3000;
	switch(him->s_ef.a_str) {
		case 23: return 2000;
		case 22: return 1500;
		case 21: return 1250;
		case 20: return 1100;
		case 19: return 1000;
		case 18: return 700;
		case 17: return 500;
		case 16: return 350;
		case 15:
		case 14: return 200;
		case 13:
		case 12: return 100;
		case 11:
		case 10:
		case  9:
		case  8: return 0;
		case  7:
		case  6: return -150;
		case  5:
		case  4: return -250;
	}
	return -350;
}


/*
 * totalenc:
 *	Get total weight that the hero can carry
 */
totalenc()
{
	reg int wtotal;

	wtotal = NORMENCB + playenc();
	switch(hungry_state) {
		case F_OKAY:
		case F_HUNGRY:	;						/* no change */
		when F_WEAK:	wtotal -= wtotal / 10;	/* 10% off weak */
		when F_FAINT:	wtotal /= 2;			/* 50% off faint */
	}
	return wtotal;
}

/*
 * whgtchk:
 *	See if the hero can carry his pack
 */
wghtchk(fromfuse)
int fromfuse;
{
	reg int dropchk, err = TRUE;
	reg char ch;

	inwhgt = TRUE;
	if (him->s_pack > him->s_carry) {
		ch = player.t_oldch;
		extinguish(wghtchk);
		if ((ch != FLOOR && ch != PASSAGE) || isfight) {
			fuse(wghtchk, TRUE, 1);
			inwhgt = FALSE;
			return;
		}
		msg("Your pack is too heavy for you.");
		do {
			dropchk = drop(NULL);
			if (dropchk == SOMTHERE)
				err = FALSE;
			else if (dropchk == FALSE) {
				mpos = 0;
				msg("You must drop something");
			}
			if (dropchk == TRUE)
				err = FALSE;
		} while(err);
	}
	inwhgt = FALSE;
}


/*
 * hitweight:
 *	Gets the fighting ability according to current weight
 * 	This returns a  +1 hit for light pack weight
 * 			 0 hit for medium pack weight
 *			-1 hit for heavy pack weight
 */
hitweight()
{
	return(2 - foodlev);
}
