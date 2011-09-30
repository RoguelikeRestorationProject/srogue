/*
 * Do anything associated with a new dungeon level
 *
 * @(#)new_level.c	9.0	(rdk)	 7/17/84
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
 * new_level:
 *	Dig and draw a new level 
 */
new_level(ltype)
int ltype;
{
	register i;
	register char ch;
	struct coord traploc;
	struct room *rp;

	if (level > max_level)
		max_level = level;

	wclear(cw);
	wclear(mw);
	clear();

	isfight = FALSE;
	levtype = ltype;

	free_list(mlist);			/* free monster list */

	if (levtype == POSTLEV)
		do_post();
	else {
		lev_mon();			/* fill in monster list */

		if (levtype == MAZELEV)
			do_maze();
		else {				/* normal levels */
			do_rooms();		/* Draw rooms */
			do_passages();		/* Draw passages */
		}
		no_food++;
		put_things();			/* Place objects (if any) */
	}
	/*
	 * Place the staircase down.
	 */
	stairs = *rnd_pos(&rooms[rnd_room()]);
	mvaddch(stairs.y, stairs.x, STAIRS);
	ntraps = 0;

	if (levtype == NORMLEV)
	{
		struct trap *trp, *maxtrp;

		/* Place the traps for normal levels only */

		if (rnd(10) < level)
		{
			ntraps = rnd(level / 4) + 1;

			if (ntraps > MAXTRAPS)
				ntraps = MAXTRAPS;

			maxtrp = &traps[ntraps];
			for (trp = &traps[0]; trp < maxtrp; trp++)
			{
again:
				switch(rnd(TYPETRAPS + 1))
				{
					case 0:
						if (rnd(100) > 25)
							goto again;
						else
							ch = POST;

					when 1: ch = TRAPDOOR;
					when 2: ch = BEARTRAP;
					when 3: ch = SLEEPTRAP;
					when 4: ch = ARROWTRAP;
					when 5: ch = TELTRAP;
					when 6: ch = DARTTRAP;
					when 7: ch = MAZETRAP;
					when 8:
					case 9:
						if (rnd(100) > 80)
							goto again;
						else
							ch = POOL;
				}
				trp->tr_flags = 0;
				traploc = *rnd_pos(&rooms[rnd_room()]);
				mvaddch(traploc.y,traploc.x,ch);
				trp->tr_type = ch;
				trp->tr_pos = traploc;

				if (ch == POOL || ch == POST)
					trp->tr_flags |= ISFOUND;

				if (ch==TELTRAP && rnd(100)<20 && trp<maxtrp-1)
				{
					struct coord newloc;

					newloc = *rnd_pos(&rooms[rnd_room()]);
					trp->tr_goto = newloc;
					trp++;
					trp->tr_goto = traploc;
					trp->tr_type = TELTRAP;
					trp->tr_pos = newloc;
					mvaddch(newloc.y, newloc.x, TELTRAP);
				}
				else
					trp->tr_goto = rndspot;
			}
		}
	}
	do
	{
		rp = &rooms[rnd_room()];
		hero = *rnd_pos(rp);
	} while(levtype==MAZELEV&&DISTANCE(hero.y,hero.x,stairs.y,stairs.x)<10);

	player.t_room = rp;
	player.t_oldch = mvinch(hero.y, hero.x);
	light(&hero);
	mvwaddch(cw,hero.y,hero.x,PLAYER);
	nochange = FALSE;
}


/*
 * rnd_room:
 *	Pick a room that is really there
 */
rnd_room()
{
	register rm;

	if (levtype != NORMLEV)
		rm = 0;
	else
	{
		do {
			rm = rnd(MAXROOMS);
		} while (rf_on(&rooms[rm],ISGONE));
	}
	return rm;
}


/*
 * put_things:
 *	put potions and scrolls on this level
 */

put_things()
{
	register i, cnt, rm;
	struct linked_list *item;
	struct object *cur;
	struct coord tp;

	/* Throw away stuff left on the previous level (if anything) */

	free_list(lvl_obj);

	/* The only way to get new stuff is to go down into the dungeon. */

	if (goingup())
		return;

	/* Do MAXOBJ attempts to put things on a level */

	for (i = 0; i < MAXOBJ; i++)
	{
		if (rnd(100) < 40)
		{
			item = new_thing(FALSE, ANYTHING);
			attach(lvl_obj, item);
			cur = OBJPTR(item);
			cnt = 0;
			do {
				/* skip treasure rooms */
				rm = rnd_room();
				if (++cnt > 500)
					break;
			} while(rf_on(&rooms[rm],ISTREAS) && levtype!=MAZELEV);

			tp = *rnd_pos(&rooms[rm]);
			mvaddch(tp.y, tp.x, cur->o_type);
			cur->o_pos = tp;
		}
	}
	/*
	 * If he is really deep in the dungeon and he hasn't found the
	 * amulet yet, put it somewhere on the ground
	 */
	if (level >= AMLEVEL && !amulet && rnd(100) < 70)
	{
		item = new_thing(FALSE, AMULET, 0);
		attach(lvl_obj, item);
		cur = OBJPTR(item);
		rm = rnd_room();
		tp = *rnd_pos(&rooms[rm]);
		mvaddch(tp.y, tp.x, cur->o_type);
		cur->o_pos = tp;
	}

	for (i = 0; i < MAXROOMS; i++)		/* loop through all */
	{
		if (rf_on(&rooms[i],ISTREAS))	/* treasure rooms */
		{
			int numthgs, isfood;

			numthgs = rnd(level / 3) + 6;
			while (numthgs-- >= 0)
			{
				isfood = TRUE;
				do {
					item = new_thing(TRUE, ANYTHING);
					cur = OBJPTR(item);

					/* dont create food for */
					if (cur->o_type == FOOD)
						discard(item);

					/* treasure rooms */
					else
						isfood = FALSE;

				} while (isfood);

				attach(lvl_obj, item);
				tp = *rnd_pos(&rooms[i]);
				mvaddch(tp.y, tp.x, cur->o_type);
				cur->o_pos = tp;
			}
		}
	}
}
