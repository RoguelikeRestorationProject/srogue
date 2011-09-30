/*
 * all sorts of miscellaneous routines
 *
 * @(#)misc.c	9.0	(rdk)	 7/17/84
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
 * waste_time:
 *	Do nothing but let other things happen
 */
waste_time()
{
	if (inwhgt)		/* if from wghtchk, then done */
	     return;
	do_daemons(BEFORE);
	do_daemons(AFTER);
	do_fuses();
}

/*
 * getindex:
 *	Convert a type into an index for the things structures
 */
getindex(what)
char what;
{
	int index = -1;

	switch (what) {
		case POTION:	index = TYP_POTION;
		when SCROLL:	index = TYP_SCROLL;
		when FOOD:		index = TYP_FOOD;
		when RING:		index = TYP_RING;
		when AMULET:	index = TYP_AMULET;
		when ARMOR:		index = TYP_ARMOR;
		when WEAPON:	index = TYP_WEAPON;
		when STICK:		index = TYP_STICK;
	}
	return index;
}

/*
 * tr_name:
 *	print the name of a trap
 */
char *
tr_name(ch)
char ch;
{
	reg char *s;

	switch (ch) {
		case TRAPDOOR:
			s = "A trapdoor.";
		when BEARTRAP:
			s = "A beartrap.";
		when SLEEPTRAP:
			s = "A sleeping gas trap.";
		when ARROWTRAP:
			s = "An arrow trap.";
		when TELTRAP:
			s = "A teleport trap.";
		when DARTTRAP:
			s = "A dart trap.";
		when POOL:
			s = "A magic pool.";
		when POST:
			s = "A trading post.";
		when MAZETRAP:
			s = "A maze trap.";
	otherwise:
		s = "A bottomless pit.";		/* shouldn't get here */
	}
	return s;
}

/*
 * Look:
 *	A quick glance all around the player
 */
look(wakeup)
bool wakeup;
{
	reg char ch;
	reg int oldx, oldy, y, x;
	reg struct room *rp;
	int ey, ex, oex, oey;
	int passcount = 0;
	bool inpass, blind;

	getyx(cw, oldy, oldx);
	oex = player.t_oldpos.x;
	oey = player.t_oldpos.y;
	blind = pl_on(ISBLIND);
	if ((oldrp != NULL && rf_on(oldrp,ISDARK)) || blind) {
		for (x = oex - 1; x <= oex + 1; x += 1)
			for (y = oey - 1; y <= oey + 1; y += 1)
				if ((y != hero.y || x != hero.x) && show(y, x) == FLOOR)
					mvwaddch(cw, y, x, ' ');
	}
	rp = player.t_room;
	inpass = (rp == NULL);				/* TRUE when not in a room */
	ey = hero.y + 1;
	ex = hero.x + 1;
	for (x = hero.x - 1; x <= ex; x += 1) {
		if (x >= 0 && x <= COLS - 1) {
			for (y = hero.y - 1; y <= ey; y += 1) {
				if (y <= 0 || y >= LINES - 2)
					continue;
				if (isalpha(mvwinch(mw, y, x))) {
					reg struct linked_list *it;
					reg struct thing *tp;

					if (wakeup || (!inpass && rf_on(rp, ISTREAS)))
						it = wake_monster(y, x);
					else
						it = find_mons(y, x);
					if (it == NULL)				/* lost monster */
						mvaddch(y, x, FLOOR);
					else {
						tp = THINGPTR(it);
						if (isatrap(tp->t_oldch = mvinch(y, x))) {
							struct trap *trp;

							if ((trp = trap_at(y,x)) == NULL)
								break;
							if (trp->tr_flags & ISFOUND)
								tp->t_oldch = trp->tr_type;
							else
								tp->t_oldch = FLOOR;
						}
						if (tp->t_oldch == FLOOR && rf_on(rp,ISDARK))
							if (!blind)
								tp->t_oldch = ' ';
					}
				}
				/*
				 * Secret doors show as walls
				 */
				if ((ch = show(y, x)) == SECRETDOOR) {
					if (inpass || y == rp->r_pos.y || y == rp->r_pos.y + rp->r_max.y - 1)
						ch = '-';
					else
						ch = '|';
				}
				/*
				 * Don't show room walls if he is in a passage
				 */
				if (!blind) {
					if ((y == hero.y && x == hero.x) || (inpass && (ch == '-' || ch == '|')))
						continue;
				}
				else
					ch = ' ';
				wmove(cw, y, x);
				waddch(cw, ch);
				if (door_stop && !firstmove && running) {
					switch (runch) {
						case 'h':
							if (x == ex)
								continue;
						when 'j':
							if (y == hero.y - 1)
								continue;
						when 'k':
							if (y == ey)
								continue;
						when 'l':
							if (x == hero.x - 1)
								continue;
						when 'y':
							if ((x + y) - (hero.x + hero.y) >= 1)
								continue;
						when 'u':
							if ((y - x) - (hero.y - hero.x) >= 1)
								continue;
						when 'n':
							if ((x + y) - (hero.x + hero.y) <= -1)
								continue;
						when 'b':
							if ((y - x) - (hero.y - hero.x) <= -1)
								continue;
					}
					switch (ch) {
						case DOOR:
							if (x == hero.x || y == hero.y)
								running = FALSE;
							break;
						case PASSAGE:
							if (x == hero.x || y == hero.y)
								passcount += 1;
							break;
						case FLOOR:
						case '|':
						case '-':
						case ' ':
							break;
						default:
							running = FALSE;
							break;
					}
				}
			}
		}
	}
	if (door_stop && !firstmove && passcount > 1)
		running = FALSE;
	mvwaddch(cw, hero.y, hero.x, PLAYER);
	wmove(cw, oldy, oldx);
	player.t_oldpos = hero;
	oldrp = rp;
}

/*
 * find_obj:
 *	find the unclaimed object at y, x
 */
struct linked_list *
find_obj(y, x)
int y, x;
{
	reg struct linked_list *obj;
	reg struct object *op;

	for (obj = lvl_obj; obj != NULL; obj = next(obj)) {
		op = OBJPTR(obj);
		if (op->o_pos.y == y && op->o_pos.x == x)
			return obj;
	}
	return NULL;
}

/*
 * eat:
 *	Let the hero eat some food.
 */
eat()
{
	reg struct linked_list *item;
	reg struct object *obj;
	reg int goodfood, cursed;

	if ((item = get_item("eat", FOOD)) == NULL)
		return;
	obj = OBJPTR(item);
	if (obj->o_type != FOOD) {
		msg("That's Inedible!");
		after = FALSE;
		return;
	}
	cursed = 1;
	if (o_on(obj, ISCURSED))
		cursed += 1;
	else if (o_on(obj, ISBLESS))
		cursed -= 1;
	if (obj->o_which == FRUITFOOD) {
		msg("My, that was a yummy %s.", fruit);
		goodfood = 100;
	}
	else {
		if (rnd(100) > 80 || o_on(obj, ISCURSED)) {
			msg("Yuk, this food tastes like ARA.");
			goodfood = 300;
			him->s_exp += 1;
			check_level();
		}
		else {
			msg("Yum, that tasted good.");
			goodfood = 200;
		}
	}
	goodfood *= cursed;
	if ((food_left += HUNGERTIME + rnd(400) - goodfood) > STOMACHSIZE)
		food_left = STOMACHSIZE;
	hungry_state = F_OKAY;
	updpack();					/* update pack */
	if (obj == cur_weapon)
		cur_weapon = NULL;
	del_pack(item);		/* get rid of the food */
}

/*
 * aggravate:
 *	aggravate all the monsters on this level
 */
aggravate()
{
	reg struct linked_list *mi;

	for (mi = mlist; mi != NULL; mi = next(mi))
		runto(&(THINGPTR(mi))->t_pos, &hero);
}

/*
 * vowelstr:
 * 	If string starts with a vowel, return "n" for an "an"
 */
char *
vowelstr(str)
char *str;
{
	switch (tolower(*str)) {
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
			return "n";
		default:
			return "";
	}
}

/* 
 * is_current:
 *	See if the object is one of the currently used items
 */
is_current(obj)
struct object *obj;
{
	if (obj == NULL)
		return FALSE;
	if (obj == cur_armor || obj == cur_weapon || obj == cur_ring[LEFT]
	  || obj == cur_ring[RIGHT]) {
		msg("Already in use.");
		return TRUE;
	}
	return FALSE;
}

/*
 * get_dir:
 *	Set up the direction coordinates
 */
get_dir()
{
	reg char *prompt;
	reg bool gotit;

	prompt = "Direction: ";
	do {
		gotit = TRUE;
		switch (readchar()) {
			case 'h': case'H': delta.y =  0; delta.x = -1;
			when 'j': case'J': delta.y =  1; delta.x =  0;
			when 'k': case'K': delta.y = -1; delta.x =  0;
			when 'l': case'L': delta.y =  0; delta.x =  1;
			when 'y': case'Y': delta.y = -1; delta.x = -1;
			when 'u': case'U': delta.y = -1; delta.x =  1;
			when 'b': case'B': delta.y =  1; delta.x = -1;
			when 'n': case'N': delta.y =  1; delta.x =  1;
			when ESCAPE: return FALSE;
			otherwise:
				mpos = 0;
				msg(prompt);
				gotit = FALSE;
		}
	} until (gotit);
	if (pl_on(ISHUH) && rnd(100) > 80) {
		do {
			delta.y = rnd(3) - 1;
			delta.x = rnd(3) - 1;
		} while (delta.y == 0 && delta.x == 0);
	}
	mpos = 0;
	return TRUE;
}

/*
 * initfood:
 *	Set up stuff for a food-type object
 */
initfood(what)
struct object *what;
{
	what->o_type = FOOD;
	what->o_group = NORMFOOD;
	if (rnd(100) < 15)
		what->o_group = FRUITFOOD;
	what->o_which = what->o_group;
	what->o_count = 1 + extras();
	what->o_flags = ISKNOW;
	what->o_weight = things[TYP_FOOD].mi_wght;
	what->o_typname = things[TYP_FOOD].mi_name;
	what->o_hplus = what->o_dplus = 0;
	what->o_vol = itemvol(what);
}
