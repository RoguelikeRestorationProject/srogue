/*
 * Draw the nine rooms on the screen
 *
 * @(#)rooms.c	9.0	(rdk)	 7/17/84
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
 * do_rooms:
 *	Place the rooms in the dungeon
 */
do_rooms()
{
	int mloops, mchance, nummons, left_out, roomtries;
	bool treas = FALSE;
	reg int i;
	reg struct room *rp;
	reg struct linked_list *item;
	reg struct thing *tp;
	struct coord top, bsze, mp;

	/*
	 * bsze is the maximum room size
	 */
	bsze.x = COLS / 3;
	bsze.y = (LINES - 1) / 3;
	/*
	 * Clear things for a new level
	 */
	for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
		rp->r_goldval = rp->r_nexits = rp->r_flags = 0;
	/*
	 * Put the gone rooms, if any, on the level
	 */
	left_out = rnd(4);
	for (i = 0; i < left_out; i++)
		rooms[rnd_room()].r_flags |= ISGONE;
	/*
	 * dig and populate all the rooms on the level
	 */
	for (i = 0, rp = rooms; i < MAXROOMS; rp++, i++) {
		/*
		 * Find upper left corner of box that this room goes in
		 */
		top.x = (i%3) * bsze.x + 1;
		top.y = i/3 * bsze.y;
		if (rf_on(rp,ISGONE)) {
			/*
			 * Place a gone room.  Make certain that there is a
			 * blank line for passage drawing.
			 */
			roomtries = 0;
			do {
				rp->r_pos.x = top.x + rnd(bsze.x-2) + 1;
				rp->r_pos.y = top.y + rnd(bsze.y-2) + 1;
				rp->r_max.x = -COLS;
				rp->r_max.x = -LINES;
				if (++roomtries > 250)
					fatal("failed to place a gone room");
			} until(rp->r_pos.y > 0 && rp->r_pos.y < LINES-2);
			continue;
		}
		if (rnd(10) < level-1)
			rp->r_flags |= ISDARK;
		/*
		 * Find a place and size for a random room
		 */
		roomtries = 0;	
		do {
			rp->r_max.x = rnd(bsze.x - 4) + 4;
			rp->r_max.y = rnd(bsze.y - 4) + 4;
			rp->r_pos.x = top.x + rnd(bsze.x - rp->r_max.x);
			rp->r_pos.y = top.y + rnd(bsze.y - rp->r_max.y);
			if (++roomtries > 250) {
				fatal("failed to place a good room");
			}
		} until (rp->r_pos.y != 0);
		if (level < max_level)
			mchance = 30;	/* 30% when going up (all monsters) */
		else
			mchance = 3;	/* 3% when going down */
		treas = FALSE;
		if (rnd(100) < mchance && (rp->r_max.x * rp->r_max.y) >
		  ((bsze.x * bsze.y * 55) / 100)) {
			treas = TRUE;
			rp->r_flags |= ISTREAS;
			rp->r_flags |= ISDARK;
		}
		/*
		 * Put the gold in
		 */
		if ((rnd(100) < 50 || treas) && (!amulet || level >= max_level)) {
			rp->r_goldval = GOLDCALC;
			if (treas)
				rp->r_goldval += 200 + (15 * (rnd(level) + 2));
			rp->r_gold.y = rp->r_pos.y + rnd(rp->r_max.y - 2) + 1;
			rp->r_gold.x = rp->r_pos.x + rnd(rp->r_max.x - 2) + 1;
		}
		draw_room(rp);
		/*
		 * Put the monster in
		 */
		if (treas) {
			mloops = rnd(level / 3) + 6;
			mchance = 1;
		}
		else {
			mloops = 1;
			mchance = 100;
		}
		for (nummons = 0; nummons < mloops; nummons++) {
			if (rnd(mchance) < (rp->r_goldval > 0 ? 80 : 25))
				add_mon(rp, treas);
		}
	}
}

/*
 * add_mon:
 *	Add a monster to a room
 */
add_mon(rm, treas)
struct room *rm;
bool treas;
{
	reg struct thing *tp;
	reg struct linked_list *item;
	struct coord mp;
	int chance;

	mp = *rnd_pos(rm);
	item = new_monster(rnd_mon(FALSE,FALSE), &mp, treas);
	tp = THINGPTR(item);
	chance = rnd(100);
	if (levtype == MAZELEV)
		chance = rnd(50);
	/*
	 * See if monster has a treasure
	 */
	if (levtype == MAZELEV && rnd(100) < 20) {
		reg struct linked_list *fd;

		fd = new_thing(FALSE, FOOD, 0);
		attach(tp->t_pack, fd);
	}
	else {
		if (chance < monsters[tp->t_indx].m_carry)
			attach(tp->t_pack, new_thing(FALSE, ANYTHING));
	}
}

/*
 * draw_room:
 *	Draw a box around a room
 */
draw_room(rp)
struct room *rp;
{
	reg int j, k;

	move(rp->r_pos.y, rp->r_pos.x+1);
	vert(rp->r_max.y-2);			/* Draw left side */
	move(rp->r_pos.y+rp->r_max.y-1, rp->r_pos.x);
	horiz(rp->r_max.x);				/* Draw bottom */
	move(rp->r_pos.y, rp->r_pos.x);
	horiz(rp->r_max.x);				/* Draw top */
	vert(rp->r_max.y-2);			/* Draw right side */
	/*
	 * Put the floor down
	 */
	for (j = 1; j < rp->r_max.y - 1; j++) {
		move(rp->r_pos.y + j, rp->r_pos.x + 1);
		for (k = 1; k < rp->r_max.x - 1; k++) {
			addch(FLOOR);
		}
	}
	/*
	 * Put the gold there
	 */
	if (rp->r_goldval > 0)
		mvaddch(rp->r_gold.y, rp->r_gold.x, GOLD);
}

/*
 * horiz:
 *	draw a horizontal line
 */
horiz(cnt)
int cnt;
{
	while (cnt-- > 0)
	addch('-');
}


/*
 * vert:
 *	draw a vertical line
 */
vert(cnt)
int cnt;
{
	reg int x, y;

	getyx(stdscr, y, x);
	x--;
	while (cnt-- > 0) {
		move(++y, x);
		addch('|');
	}
}


/*
 * rnd_pos:
 *	pick a random spot in a room
 */
struct coord *
rnd_pos(rp)
struct room *rp;
{
	reg int y, x, i;
	static struct coord spot;

	i = 0;
	do {
		x = rp->r_pos.x + rnd(rp->r_max.x - 2) + 1;
		y = rp->r_pos.y + rnd(rp->r_max.y - 2) + 1;
		i += 1;
	} while(winat(y, x) != FLOOR && i < 1000);
	spot.x = x;
	spot.y = y;
	return &spot;
}

/*
 * rf_on:
 * 	Returns TRUE if flag is set for room stuff
 */
rf_on(rm, bit)
struct room *rm;
long bit;
{
	return (rm->r_flags & bit);
}
