/*
 * This file contains misc functions for dealing with armor
 *
 * @(#)armor.c	9.0	(rdk)	 7/17/84
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
 * wear:
 *	The player wants to wear something, so let the hero try
 */
wear()
{
	reg struct linked_list *item;
	reg struct object *obj;

	if (cur_armor != NULL) {
		msg("You are already wearing some.");
		after = FALSE;
		return;
	}
	if ((item = get_item("wear", ARMOR)) == NULL)
		return;
	obj = OBJPTR(item);
	if (obj->o_type != ARMOR) {
		msg("You can't wear that.");
		return;
	}
	waste_time();
	msg("Wearing %s.", a_magic[obj->o_which].mi_name);
	cur_armor = obj;
	setoflg(obj,ISKNOW);
	nochange = FALSE;
}


/*
 * take_off:
 *	Get the armor off of the players back
 */
take_off()
{
	reg struct object *obj;

	if ((obj = cur_armor) == NULL) {
		msg("Not wearing any armor.");
		return;
	}
	if (!dropcheck(cur_armor))
		return;
	cur_armor = NULL;
	msg("Was wearing %c) %s",pack_char(obj),inv_name(obj,TRUE));
	nochange = FALSE;
}

/*
 * initarmor:
 *		Initialize some armor.
 */
initarmor(obj, what)
struct object *obj;
int what;
{
	struct init_armor *iwa;
	struct magic_item *mi;

	obj->o_type = ARMOR;
	obj->o_which = what;
	iwa = &armors[what];
	mi = &a_magic[what];
	obj->o_vol = iwa->a_vol;
	obj->o_ac = iwa->a_class;
	obj->o_weight = iwa->a_wght;
	obj->o_typname = things[TYP_ARMOR].mi_name;
}

/*
 * hurt_armor:
 *	Returns TRUE if armor is damaged
 */
hurt_armor(obj)
struct object *obj;
{
	reg int type, ac;

	if (obj != NULL) {
		if (o_on(obj, ISPROT) || (o_on(obj, ISBLESS) && rnd(100) < 10))
			return FALSE;
		ac = obj->o_ac;
		type = obj->o_which;
		if (type != PADDED && type != LEATHER)
			if ((type == STUDDED && ac < 8) || (type != STUDDED && ac < 9))
				return TRUE;
	}
	return FALSE;
}
