/*
 * Contains functions for dealing with things like
 * potions and scrolls
 *
 * @(#)things.c	9.0	(rdk)	 7/17/84
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
 * inv_name:
 *	Return the name of something as it would appear in an inventory.
 */
char *
inv_name(obj, drop)
struct object *obj;
bool drop;
{
	reg char *pb, *tn, *pl;
	reg int wh, knowit;
	char nm[3], *inm, *q;

	wh = obj->o_which;
	knowit = FALSE;
	if (obj->o_count > 1)
		pl = "s";
	else
		pl = "";
	if (obj->o_count > 1)
		sprintf(nm, "%d", obj->o_count);
	else
		strcpy(nm, "A");
	tn = obj->o_typname;
	q = "";
	switch(obj->o_type) {
	case SCROLL:
		sprintf(prbuf, "%s %s%s ", nm, tn, pl);
		pb = &prbuf[strlen(prbuf)];
		if (s_know[wh] || o_on(obj,ISPOST)) {
			knowit = TRUE;
			sprintf(pb, "of %s", s_magic[wh].mi_name);
		}
		else if (s_guess[wh])
			sprintf(pb, "called %s", s_guess[wh]);
		else
			sprintf(pb, "titled '%s'", s_names[wh]);
    when POTION:
		sprintf(prbuf, "%s %s%s ", nm, tn, pl);
		pb = &prbuf[strlen(prbuf)];
		if (p_know[wh] || o_on(obj, ISPOST)) {
			sprintf(pb, "of %s", p_magic[wh].mi_name);
			knowit = TRUE;
			if (p_know[wh]) {
				pb = &prbuf[strlen(prbuf)];
				sprintf(pb,"(%s)",p_colors[wh]);
			}
		}
		else if (p_guess[wh])
			sprintf(pb,"called %s(%s)", p_guess[wh],p_colors[wh]);
		else
			sprintf(prbuf,"%s%s %s %s%s", nm, vowelstr(p_colors[wh]),
				p_colors[wh], tn, pl);
	when FOOD:
		if (wh == 1) {
			if (obj->o_count == 1)
				q = vowelstr(fruit);
			sprintf(prbuf, "%s%s %s%s", nm, q, fruit, pl);
		}
		else {
			if (obj->o_count == 1)
				sprintf(prbuf, "Some %s", tn);
			else
				sprintf(prbuf, "%s rations of %s", nm, tn);
		}
		knowit = TRUE;
	when WEAPON:
		inm = w_magic[wh].mi_name;
		strcpy(prbuf, nm);
		if (obj->o_count == 1)
			q = vowelstr(inm);
		pb = &prbuf[strlen(prbuf)];
		if (o_on(obj,ISKNOW | ISPOST)) {
			knowit = TRUE;
			sprintf(pb, " %s %s", num(obj->o_hplus, obj->o_dplus), inm);
		}
		else
			sprintf(pb, "%s %s", q, inm);
		strcat(prbuf, pl);
	when ARMOR:
		inm = a_magic[wh].mi_name;
		if (o_on(obj,ISKNOW | ISPOST)) {
			knowit = TRUE;
			sprintf(prbuf, "%s %s",num(armors[wh].a_class - obj->o_ac, 0),
				inm);
		}
		else
			sprintf(prbuf, "%s", inm);
	when AMULET:
		strcpy(prbuf, "The Amulet of Yendor");
	when STICK: {
		struct rod *rd;

		rd = &ws_stuff[wh];
		sprintf(prbuf, "A %s ", rd->ws_type);
		pb = &prbuf[strlen(prbuf)];
		if (ws_know[wh] || o_on(obj, ISPOST)) {
			knowit = TRUE;
			sprintf(pb,"of %s%s",ws_magic[wh].mi_name,charge_str(obj));
			if (ws_know[wh]) {
				pb = &prbuf[strlen(prbuf)];
				sprintf(pb,"(%s)",rd->ws_made);
			}
		}
		else if (ws_guess[wh])
			sprintf(pb, "called %s(%s)", ws_guess[wh], rd->ws_made);
		else
			sprintf(prbuf, "A%s %s %s", vowelstr(rd->ws_made),
				rd->ws_made, rd->ws_type);
	}
    when RING:
		if (r_know[wh] || o_on(obj, ISPOST)) {
			knowit = TRUE;
			sprintf(prbuf, "A%s %s of %s", ring_num(obj), tn,
			  r_magic[wh].mi_name);
			if (r_know[wh]) {
				pb = &prbuf[strlen(prbuf)];
				sprintf(pb,"(%s)", r_stones[wh]);
			}
		}
		else if (r_guess[wh])
			sprintf(prbuf,"A %s called %s(%s)",tn, r_guess[wh],
				r_stones[wh]);
		else
			sprintf(prbuf,"A%s %s %s",vowelstr(r_stones[wh]), 
				r_stones[wh], tn);
	otherwise:
		sprintf(prbuf,"Something bizarre %s", unctrl(obj->o_type));
	}
	if (obj == cur_armor)
		strcat(prbuf, " (being worn)");
	if (obj == cur_weapon)
		strcat(prbuf, " (weapon in hand)");
	if (obj == cur_ring[LEFT])
		strcat(prbuf, " (on left hand)");
	else if (obj == cur_ring[RIGHT])
		strcat(prbuf, " (on right hand)");
	if (drop && isupper(prbuf[0]))
		prbuf[0] = tolower(prbuf[0]);
	else if (!drop && islower(*prbuf))
		*prbuf = toupper(*prbuf);
	if (o_on(obj, ISPROT))
		strcat(prbuf, " [!]");
	if (o_on(obj, ISPOST))
		strcat(prbuf, " [$]");
	if (knowit) {
		if (o_on(obj, ISCURSED))
			strcat(prbuf, " [-]");
		else if (o_on(obj, ISBLESS))
			strcat(prbuf, " [+]");
	}
	if (!drop)
		strcat(prbuf, ".");
	return prbuf;
}

/*
 * money:
 *	Add to characters purse
 */
money()
{
	reg struct room *rp;
	reg struct linked_list *item;
	reg struct thing *tp;

	rp = player.t_room;
	if (rp != NULL && ce(hero, rp->r_gold)) {
		msg("%d gold pieces.", rp->r_goldval);
		purse += rp->r_goldval;
		rp->r_goldval = 0;
		cmov(rp->r_gold);
		addch(FLOOR);
		/*
		 * once gold is taken, all monsters will chase him
		 */
		for (item = mlist; item != NULL; item = next(item)) {
			tp = THINGPTR(item);
			if (rnd(100) < 70 && tp->t_room == rp && !iswearing(R_STEALTH)
			  && ((tp->t_flags & (ISMEAN | ISGREED)) || rnd(1000) < 20))
				runto(&tp->t_pos, &hero);
		}
	}
	else
		msg("That gold must have been counterfeit.");
}


/*
 * drop:
 *	put something down
 */
drop(item)
struct linked_list *item;
{
	reg char ch;
	reg struct linked_list *ll, *nll;
	reg struct object *op;

	if (item == NULL) {
		ch = mvinch(hero.y, hero.x);
		if (ch != FLOOR && ch != PASSAGE && ch != POOL) {
			msg("There is something there already.");
			after = FALSE;
			return SOMTHERE;
		}
		if ((ll = get_item("drop", 0)) == NULL)
			return FALSE;
	}
	else {
		ll = item;
	}
	op = OBJPTR(ll);
	if (!dropcheck(op))
		return CANTDROP;
	/*
	 * Take it out of the pack
	 */
	if (op->o_count >= 2 && op->o_type != WEAPON) {
		nll = new_item(sizeof *op);
		op->o_count--;
		op->o_vol = itemvol(op);
		op = OBJPTR(nll);
		*op = *(OBJPTR(ll));
		op->o_count = 1;
		op->o_vol = itemvol(op);
		ll = nll;
	}
	else {
		detach(pack, ll);
	}
	if (ch == POOL) {
		msg("%s sinks out of sight.",inv_name(op, TRUE));
		discard(ll);
	}
	else {			/* put on dungeon floor */
		if (levtype == POSTLEV) {
			op->o_pos = hero;	/* same place as hero */
			fall(ll,FALSE);
			if (item == NULL)	/* if item wasn't sold */
				msg("Thanks for your donation to the Fiend's flea market.");
		}
		else {
			attach(lvl_obj, ll);
			mvaddch(hero.y, hero.x, op->o_type);
			op->o_pos = hero;
			msg("Dropped %s", inv_name(op, TRUE));
		}
	}
	updpack();			/* new pack weight */
	return TRUE;
}


/*
 * dropcheck:
 *	Do special checks for dropping or unweilding|unwearing|unringing
 */
dropcheck(op)
struct object *op;
{
	if (op == NULL)
		return TRUE;
	if (levtype == POSTLEV) {
		if (o_on(op,ISCURSED) && o_on(op,ISKNOW)) {
			msg("The trader does not accept shoddy merchandise.");
			return FALSE;
		}
		else {
			cur_null(op);	/* update cur_weapon, etc */
			return TRUE;
		}
	}
	if (op != cur_armor && op != cur_weapon
	  && op != cur_ring[LEFT] && op != cur_ring[RIGHT])
		return TRUE;
	if (o_on(op,ISCURSED)) {
		msg("You can't.  It appears to be cursed.");
		return FALSE;
	}
	if (op == cur_weapon)
		cur_weapon = NULL;
	else if (op == cur_armor) {
		waste_time();
		cur_armor = NULL;
	}
	else if (op == cur_ring[LEFT] || op == cur_ring[RIGHT])
		toss_ring(op);
	return TRUE;
}


/*
 * new_thing:
 *	Return a new thing
 */
struct linked_list *
new_thing(treas, type, which)
int type, which;
bool treas;
{
	struct linked_list *item;
	struct magic_item *mi;
	struct object *cur;
	int chance, whi;

	item = new_item(sizeof *cur);
	cur = OBJPTR(item);
	basic_init(cur);
	if (type == DONTCARE) {
		if (++no_food > 4 && !treas)
			whi = TYP_FOOD;
		else
			whi = pick_one(things);
	}
	else {
		whi = getindex(type);
	}
	mi = thnginfo[whi].mf_magic;
	if (which == DONTCARE) {
		which = 0;
		if (mi != NULL)
			which = pick_one(mi);
	}
	cur->o_typname = things[whi].mi_name;
	cur->o_weight = things[whi].mi_wght;
	switch (whi) {
		case TYP_AMULET:
			cur->o_type = AMULET;
			cur->o_hplus = 500;
			strcpy(cur->o_hurldmg,"80d8");	/* if thrown, WOW!!! */
			cur->o_vol = itemvol(cur);
		when TYP_POTION:
			cur->o_type = POTION;
			cur->o_which = which;
			cur->o_count += extras();
			cur->o_vol = itemvol(cur);
		when TYP_SCROLL:
			cur->o_type = SCROLL;
			cur->o_which = which;
			cur->o_count += extras();
			cur->o_vol = itemvol(cur);
		when TYP_FOOD:
			no_food = 0;
			initfood(cur);
		when TYP_WEAPON:
			cur->o_which = which;
			init_weapon(cur, which);
			if ((chance = rnd(100)) < 10) {
				setoflg(cur,ISCURSED);
				cur->o_hplus -= rnd(3)+1;
				cur->o_dplus -= rnd(3)+1;
			}
			else if (chance < 15) {
				cur->o_hplus += rnd(3)+1;
				cur->o_dplus += rnd(3)+1;
			}
		when TYP_ARMOR:
			cur->o_which = which;
			initarmor(cur, which);
			if ((chance = rnd(100)) < 20) {
				setoflg(cur,ISCURSED);
				cur->o_ac += rnd(3)+1;
			}
			else if (chance < 30)
				cur->o_ac -= rnd(3)+1;
		when TYP_RING:
			cur->o_which = which;
			init_ring(cur, FALSE);
		when TYP_STICK:
		default:
			cur->o_which = which;
			fix_stick(cur);
	}
	return item;
}

/*
 * basic_init:
 *	Set all params of an object to the basic values.
 */
basic_init(cur)
struct object *cur;
{
	cur->o_ac = 11;
	cur->o_count = 1;
	cur->o_launch = 0;
	cur->o_typname = NULL;
	cur->o_group = newgrp();
	cur->o_weight = cur->o_vol = 0;
	cur->o_hplus = cur->o_dplus = 0;
	strcpy(cur->o_damage,"0d0");
	strcpy(cur->o_hurldmg,"0d0");
	cur->o_flags = cur->o_type = cur->o_which = 0;
}

/*
 * extras:
 *	Return the number of extra items to be created
 */
extras()
{
	reg int i;

	i = rnd(100);
	if (i < 4)			/* 4% for 2 more */
		return 2;
	else if (i < 11)	/* 7% for 1 more */
		return 1;
	else				/* otherwise no more */
		return 0;
}


/*
 * pick_one:
 * 	Pick an item out of a list of nitems possible magic items
 */
pick_one(mag)
struct magic_item *mag;
{
	reg struct magic_item *start;
	reg int i;

	start = mag;
	for (i = rnd(1000); mag->mi_name != NULL; mag++) {
		if (i < mag->mi_prob)
			break;
		if (mag->mi_name == NULL) {
			if (author() || wizard) {
				for (mag = start; mag->mi_name != NULL; mag++)
					msg("%s: %d%%", mag->mi_name, mag->mi_prob);
			}
			mag = start;
		}
	}
	return mag - start;
}
