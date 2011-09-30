/*
 * Routines to deal with the pack
 *
 * @(#)pack.c	9.0	(rdk)	 7/17/84
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
 * add_pack:
 * Pick up an object and add it to the pack.  If the argument
 * is non-null use it as the linked_list pointer instead of
 * getting it off the ground.
 */
add_pack(item, silent)
struct linked_list *item;
bool silent;
{
	reg struct linked_list *ip, *lp;
	reg struct object *obj, *op;
	bool from_floor;
	char delchar;

	if (player.t_room == NULL)
		delchar = PASSAGE;
	else
		delchar = FLOOR;
	if (item == NULL) {
		from_floor = TRUE;
		if ((item = find_obj(hero.y, hero.x)) == NULL) {
			mpos = 0;
			msg("That object must have been an illusion.");
			mvaddch(hero.y, hero.x, delchar);
			return FALSE;
		}
		/*
		 * Check for scare monster scrolls
		 */
		obj = OBJPTR(item);
		if (obj->o_type == SCROLL && obj->o_which == S_SCARE) {
			if (o_on(obj,ISFOUND)) {
				msg("The scroll turns to dust as you pick it up.");
				detach(lvl_obj, item);
				discard(item);
				mvaddch(hero.y, hero.x, delchar);	
				return FALSE;
			}
		}
	}
	else
		from_floor = FALSE;
	obj = OBJPTR(item);
	/*
	 * See if this guy can carry any more weight
	 */
	if (itemweight(obj) + him->s_pack > him->s_carry) {
		msg("You can't carry that %s.", obj->o_typname);
		return FALSE;
	}
	/*
	 * Check if there is room
	 */
	if (packvol + obj->o_vol > V_PACK) {
		msg("That %s won't fit in your pack.", obj->o_typname);
		return FALSE;
	}
	if (from_floor) {
		detach(lvl_obj, item);
		mvaddch(hero.y, hero.x, delchar);
	}
	item->l_prev = NULL;
	item->l_next = NULL;
	setoflg(obj, ISFOUND);
	/*
	 * start looking thru pack to find the start of items
	 * with the same type.
	 */
	lp = pack;
	for (ip = pack; ip != NULL; ip = next(ip)) {
		op = OBJPTR(ip);
		/*
		 * If we find a matching type then quit.
		 */
		if (op->o_type == obj->o_type)
			break;
		if (next(ip) != NULL)
			lp = next(lp);		/* update "previous" entry */
	}
	/*
	 * If the pack was empty, just stick the item in it.
	 */
	if (pack == NULL) {
		pack = item;
		item->l_prev = NULL;
	}
	/*
	 * If we looked thru the pack, but could not find an
	 * item of the same type, then stick it at the end,
	 * unless it was food, then put it in front.
	 */
	else if (ip == NULL) {
		if (obj->o_type == FOOD) {	/* insert food at front */
			item->l_next = pack;
			pack->l_prev = item;
			pack = item;
			item->l_prev = NULL;
		}
		else {						/* insert other stuff at back */
			lp->l_next = item;
			item->l_prev = lp;
		}
	}
	/*
	 * Here, we found at least one item of the same type.
	 * Look thru these items to see if there is one of the
	 * same group. If so, increment the count and throw the
	 * new item away. If not, stick it at the end of the
	 * items with the same type. Also keep all similar
	 * objects near each other, like all identify scrolls, etc.
	 */
	else {
		struct linked_list **save;

		while (ip != NULL && op->o_type == obj->o_type) {
			if (op->o_group == obj->o_group) {
				if (op->o_flags == obj->o_flags) {
					op->o_count++;
					discard(item);
					item = ip;
					goto picked_up;
				}
				else {
					goto around;
				}
			}
			if (op->o_which == obj->o_which) {
				if (obj->o_type == FOOD)
					ip = next(ip);
				break;
			}
around:
			ip = next(ip);
			if (ip != NULL) {
				op = OBJPTR(ip);
				lp = next(lp);
			}
		}
		/*
		 * If inserting into last of group at end of pack,
		 * just tack on the end.
		 */
		if (ip == NULL) {
			lp->l_next = item;
			item->l_prev = lp;
		}
		/*
		 * Insert into the last of a group of objects
		 * not at the end of the pack.
		 */
		else {
			save = &((ip->l_prev)->l_next);
			item->l_next = ip;
			item->l_prev = ip->l_prev;
			ip->l_prev = item;
			*save = item;
		}
	}
picked_up:
	obj = OBJPTR(item);
	if (!silent)
		msg("%s (%c)",inv_name(obj,FALSE),pack_char(obj));
	if (obj->o_type == AMULET)
		amulet = TRUE;
	updpack();				/* new pack weight & volume */
	return TRUE;
}

/*
 * inventory:
 *	Show what items are in a specific list
 */
inventory(list, type)
struct linked_list *list;
int type;
{
	reg struct linked_list *pc;
	reg struct object *obj;
	reg char ch;
	reg int cnt;

	if (list == NULL) {			/* empty list */
		msg(type == 0 ? "Empty handed." : "Nothing appropriate.");
		return FALSE;
	}
	else if (next(list) == NULL) {	/* only 1 item in list */
		obj = OBJPTR(list);
		msg("a) %s", inv_name(obj, FALSE));
		return TRUE;
	}
	cnt = 0;
	wclear(hw);
	for (ch = 'a', pc = list; pc != NULL; pc = next(pc), ch = npch(ch)) {
		obj = OBJPTR(pc);
		wprintw(hw,"%c) %s\n\r",ch,inv_name(obj, FALSE));
		if (++cnt > LINES - 2 && next(pc) != NULL) {
			dbotline(hw, morestr);
			cnt = 0;
			wclear(hw);
		} 
	}
	dbotline(hw,spacemsg);
	restscr(cw);
	return TRUE;
}

/*
 * pick_up:
 *	Add something to characters pack.
 */
pick_up(ch)
char ch;
{
	nochange = FALSE;
	switch(ch) {
		case GOLD:
			money();
		when ARMOR:
		case POTION:
		case FOOD:
		case WEAPON:
		case SCROLL:	
		case AMULET:
		case RING:
		case STICK:
			add_pack(NULL, FALSE);
		otherwise:
			msg("That item is ethereal !!!");
	}
}

/*
 * picky_inven:
 *	Allow player to inventory a single item
 */
picky_inven()
{
	reg struct linked_list *item;
	reg char ch, mch;

	if (pack == NULL)
		msg("You aren't carrying anything.");
	else if (next(pack) == NULL)
		msg("a) %s", inv_name(OBJPTR(pack), FALSE));
	else {
		msg("Item: ");
		mpos = 0;
		if ((mch = readchar()) == ESCAPE) {
			msg("");
			return;
		}
		for (ch='a',item=pack; item != NULL; item=next(item),ch=npch(ch))
			if (ch == mch) {
				msg("%c) %s",ch,inv_name(OBJPTR(item), FALSE));
				return;
			}
		if (ch == 'A')
			ch = 'z';
		else
			ch -= 1;
		msg("Range is 'a' to '%c'", ch);
	}
}

/*
 * get_item:
 *	pick something out of a pack for a purpose
 */
struct linked_list *
get_item(purpose, type)
char *purpose;
int type;
{
	reg struct linked_list *obj, *pit, *savepit;
	struct object *pob;
	int ch, och, anr, cnt;

	if (pack == NULL) {
		msg("You aren't carrying anything.");
		return NULL;
	}
	if (type != WEAPON && (type != 0 || next(pack) == NULL)) {
		/*
		 * see if we have any of the type requested
		 */
		pit = pack;
		anr = 0;
		for (ch = 'a'; pit != NULL; pit = next(pit), ch = npch(ch)) {
			pob = OBJPTR(pit);
			if (type == pob->o_type || type == 0) {
				++anr;
				savepit = pit;	/* save in case of only 1 */
			}
		}
		if (anr == 0) {
			msg("Nothing to %s",purpose);
			after = FALSE;
			return NULL;
		}
		else if (anr == 1) {	/* only found one of 'em */
			do {
				struct object *opb;

				opb = OBJPTR(savepit);
				msg("%s what (* for the item)?",purpose);
				och = readchar();
				if (och == '*') {
					mpos = 0;
					msg("%c) %s",pack_char(opb),inv_name(opb,FALSE));
					continue;
				}
				if (och == ESCAPE) {
					msg("");
					after = FALSE;
					return NULL;
				}
				if (isalpha(och) && och != pack_char(opb)) {
					mpos = 0;
					msg("You can't %s that !!", purpose);
					after = FALSE;
					return NULL;
				}
			} while(!isalpha(och));
			mpos = 0;
			return savepit;		/* return this item */
		}
	}
	for (;;) {
		msg("%s what? (* for list): ",purpose);
		ch = readchar();
		mpos = 0;
		if (ch == ESCAPE) {		/* abort if escape hit */
			after = FALSE;
			msg("");			/* clear display */
			return NULL;
		}
		if (ch == '*') {
			wclear(hw);
			pit = pack;		/* point to pack */
			cnt = 0;
			for (ch='a'; pit != NULL; pit=next(pit), ch=npch(ch)) {
				pob = OBJPTR(pit);
				if (type == 0 || type == pob->o_type) {
					wprintw(hw,"%c) %s\n\r",ch,inv_name(pob,FALSE));
					if (++cnt > LINES - 2 && next(pit) != NULL) {
						cnt = 0;
						dbotline(hw, morestr);
						wclear(hw);
					}
				}
			}
			wmove(hw, LINES - 1,0);
			wprintw(hw,"%s what? ",purpose);
			draw(hw);		/* write screen */
			anr = FALSE;
			do {
				ch = readchar();
				if (isalpha(ch) || ch == ESCAPE)
					anr = TRUE; 
			} while(!anr);		/* do till we got it right */
			restscr(cw);		/* redraw orig screen */
			if (ch == ESCAPE) {
				after = FALSE;
				msg("");		/* clear top line */
				return NULL;	/* all done if abort */
			}
			/* ch has item to get from pack */
		}
		for (obj=pack,och='a';obj!=NULL;obj=next(obj),och=npch(och))
			if (ch == och)
				break;
		if (obj == NULL) {
			if (och == 'A')
				och = 'z';
			else
				och -= 1;
			msg("Please specify a letter between 'a' and '%c'",och);
			continue;
		}
		else 
			return obj;
	}
}

/*
 * pack_char:
 *	Get the character of a particular item in the pack
 */
char
pack_char(obj)
struct object *obj;
{
	reg struct linked_list *item;
	reg char c;

	c = 'a';
	for (item = pack; item != NULL; item = next(item))
		if (OBJPTR(item) == obj)
			return c;
		else
			c = npch(c);
	return '%';
}

/*
 * idenpack:
 *	Identify all the items in the pack
 */
idenpack()
{
	reg struct linked_list *pc;

	for (pc = pack ; pc != NULL ; pc = next(pc))
		whatis(pc);
}


/* 
 * del_pack:
 *	Take something out of the hero's pack
 */
del_pack(what)
struct linked_list *what;
{
	reg struct object *op;

	op = OBJPTR(what);
	cur_null(op);		/* check for current stuff */
	if (op->o_count > 1) {
		op->o_count--;
	}
	else {
		detach(pack,what);
		discard(what);
	}
	updpack();
}

/*
 * cur_null:
 *	This updates cur_weapon etc for dropping things
 */
cur_null(op)
struct object *op;
{
	if (op == cur_weapon)
		cur_weapon = NULL;
	else if (op == cur_armor)
		cur_armor = NULL;
	else if (op == cur_ring[LEFT])
		cur_ring[LEFT] = NULL;
	else if (op == cur_ring[RIGHT])
		cur_ring[RIGHT] = NULL;
}
