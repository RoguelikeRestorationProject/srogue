/*
 * Functions for dealing with weapons
 *
 * @(#)weapons.c	9.0	(rdk)	 7/17/84
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
 * missile:
 *	Fire a missile in a given direction
 */
missile(ydelta, xdelta)
int ydelta, xdelta;
{
	reg struct object *obj, *nowwield;
	reg struct linked_list *item, *nitem;

	/*
	 * Get which thing we are hurling
	 */
	nowwield = cur_weapon;		/* must save current weap */
	if ((item = get_item("throw", WEAPON)) == NULL)
		return;
	obj = OBJPTR(item);
	if (!dropcheck(obj) || is_current(obj))
		return;
	if (obj == nowwield || obj->o_type != WEAPON) {
		reg int c;

		msg("Do you want to throw that %s? (y or n)",obj->o_typname);
		do {
			c = readchar();
			if (isupper(c))
				c = tolower(c);
			if (c == ESCAPE || c == 'n') {
				msg("");
				cur_weapon = nowwield;
				after = FALSE;		/* ooops, a mistake */
				return;
			}
		} while (c != 'y');	/* keep looking for good ans */
	}
	/*
	 * Get rid of the thing.  If it is a non-multiple item object, or
	 * if it is the last thing, just drop it.  Otherwise, create a new
	 * item with a count of one.
	 */
	if (obj->o_count < 2) {
		detach(pack, item);
	}
	else {
		obj->o_count--;
		obj->o_vol = itemvol(obj);
		nitem = new_item(sizeof *obj);
		obj = OBJPTR(nitem);
		*obj = *(OBJPTR(item));
		obj->o_count = 1;
		obj->o_vol = itemvol(obj);
		item = nitem;
	}
	updpack();						/* new pack weight */
	do_motion(obj, ydelta, xdelta);
	if (!isalpha(mvwinch(mw, obj->o_pos.y, obj->o_pos.x))
	  || !hit_monster(&obj->o_pos, obj))
		fall(item, TRUE);
	mvwaddch(cw, hero.y, hero.x, PLAYER);
}

/*
 * do the actual motion on the screen done by an object traveling
 * across the room
 */
do_motion(obj, ydelta, xdelta)
struct object *obj;
int ydelta, xdelta;
{
	reg int ch, y, x;

	obj->o_pos = hero;
	while (1) {
		y = obj->o_pos.y;
		x = obj->o_pos.x;
		if (!ce(obj->o_pos, hero) && cansee(unc(obj->o_pos)) &&
		  mvwinch(cw, y, x) != ' ')
			mvwaddch(cw, y, x, show(y, x));
		/*
		 * Get the new position
		 */
		obj->o_pos.y += ydelta;
		obj->o_pos.x += xdelta;
		y = obj->o_pos.y;
		x = obj->o_pos.x;
		ch = winat(y, x);
		if (step_ok(ch) && ch != DOOR) {
			if (cansee(unc(obj->o_pos)) && mvwinch(cw, y, x) != ' ') {
				mvwaddch(cw, y, x, obj->o_type);
				draw(cw);
			}
			continue;
		}
		break;
	}
}

/*
 * fall:
 *	Drop an item someplace around here.
 */

fall(item, pr)
struct linked_list *item;
bool pr;
{
	reg struct object *obj;
	reg struct room *rp;
	static struct coord fpos;

	obj = OBJPTR(item);
	if (fallpos(&obj->o_pos, &fpos, TRUE)) {
		mvaddch(fpos.y, fpos.x, obj->o_type);
		obj->o_pos = fpos;
		rp = player.t_room;
		if (rp != NULL && !rf_on(rp,ISDARK)) {
			light(&hero);
			mvwaddch(cw, hero.y, hero.x, PLAYER);
		}
		attach(lvl_obj, item);
		return;
	}

	if (pr)
        if (obj->o_type == WEAPON) /* BUGFIX: Identification trick */
            msg("Your %s vanishes as it hits the ground.", w_magic[obj->o_which].mi_name);
        else
            msg("%s vanishes as it hits the ground.", inv_name(obj,TRUE));

	discard(item);
}

/*
 * init_weapon:
 *	Set up the initial goodies for a weapon
 */

init_weapon(weap, type)
struct object *weap;
int type;
{
	reg struct init_weps *iwp;

	weap->o_type = WEAPON;
	weap->o_which = type;
	iwp = &weaps[type];
	strcpy(weap->o_damage,iwp->w_dam);
	strcpy(weap->o_hurldmg,iwp->w_hrl);
	weap->o_launch = iwp->w_launch;
	weap->o_flags = iwp->w_flags;
	weap->o_weight = iwp->w_wght;
	weap->o_typname = things[TYP_WEAPON].mi_name;
	if (o_on(weap,ISMANY))
		weap->o_count = rnd(8) + 8;
	else
		weap->o_count = 1;
	weap->o_group = newgrp();
	weap->o_vol = itemvol(weap);
}

/*
 * hit_monster:
 *	Does the missile hit the monster
 */
hit_monster(mp, obj)
struct coord *mp;
struct object *obj;
{
	return fight(mp, obj, TRUE);
}

/*
 * num:
 *	Figure out the plus number for armor/weapons
 */
char *
num(n1, n2)
int n1, n2;
{
	static char numbuf[LINLEN];

	if (n1 == 0 && n2 == 0)
		return "+0";
	if (n2 == 0)
		sprintf(numbuf, "%s%d", n1 < 0 ? "" : "+", n1);
	else
		sprintf(numbuf,"%s%d,%s%d",n1<0 ? "":"+",n1,n2<0 ? "":"+",n2);  
	return numbuf;
}

/*
 * wield:
 *	Pull out a certain weapon
 */
wield()
{
	reg struct linked_list *item;
	reg struct object *obj, *oweapon;

	oweapon = cur_weapon;
	if (!dropcheck(cur_weapon)) {
		cur_weapon = oweapon;
		return;
	}
	cur_weapon = oweapon;
	if ((item = get_item("wield", WEAPON)) == NULL)
		return;
	obj = OBJPTR(item);
	if (is_current(obj)) {
		after = FALSE;
		return;
	}
	msg("Wielding %s", inv_name(obj, TRUE));
	cur_weapon = obj;
}

/*
 * fallpos:
 *	Pick a random position around the give (y, x) coordinates
 */
fallpos(pos, newpos, passages)
struct coord *pos, *newpos;
bool passages;
{
	reg int y, x, ch;

	for (y = pos->y - 1; y <= pos->y + 1; y++) {
		for (x = pos->x - 1; x <= pos->x + 1; x++) {
			/*
			 * check to make certain the spot is empty, if it is,
			 * put the object there, set it in the level list
			 * and re-draw the room if he can see it
			 */
			if (y == hero.y && x == hero.x)
				continue;
			ch = winat(y, x);
			if (ch == FLOOR || (passages && ch == PASSAGE)) {
				newpos->y = y;
				newpos->x = x;
				return TRUE;
			}
		}
	}
	return FALSE;
}
