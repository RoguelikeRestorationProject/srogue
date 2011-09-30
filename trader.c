/*
 * Anything to do with trading posts & mazes
 *
 * @(#)trader.c	9.0	(rdk)	 7/17/84
 *
 * Super-Rogue
 * Copyright (C) 1984 Robert D. Kindelberger
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include "rogue.h"
#include "rogue.ext"

#define NOTPRICED -1

/*
 * do_post:
 *	Put a trading post room and stuff on the screen
 */
do_post()
{
	struct coord tp;
	reg int i;
	reg struct room *rp;
	reg struct object *op;
	reg struct linked_list *ll;

	free_list(lvl_obj);		/* throw old items away */

	for (rp = rooms; rp < &rooms[MAXROOMS]; rp++) {
		rp->r_goldval = 0;			/* no gold */
		rp->r_nexits = 0;			/* no exits */
		rp->r_flags = ISGONE;		/* kill all rooms */
	}
	rp = &rooms[0];					/* point to only room */
	rp->r_flags = 0;				/* this room NOT gone */
	rp->r_max.x = 40;
	rp->r_max.y = 10;				/* 10 * 40 room */
	rp->r_pos.x = (COLS - rp->r_max.x) / 2;	/* center horizontal */
	rp->r_pos.y = 1;				/* 2nd line */
	draw_room(rp);					/* draw the only room */
	i = roll(4,10);					/* 10 to 40 items */
	for (; i > 0 ; i--) {			/* place all the items */
		ll = new_thing(FALSE, ANYTHING);		/* get something */
		attach(lvl_obj, ll);
		op = OBJPTR(ll);
		setoflg(op, ISPOST);		/* object in trading post */
		tp = *rnd_pos(rp);
		op->o_pos = tp;
		mvaddch(tp.y,tp.x,op->o_type);
	}
	trader = 0;
	wmove(cw,12,0);
	waddstr(cw,"Welcome to Friendly Fiend's Flea Market\n\r");
	waddstr(cw,"=======================================\n\r");
	waddstr(cw,"$: Prices object that you stand upon.\n\r");
	waddstr(cw,"#: Buys the object that you stand upon.\n\r");
	waddstr(cw,"%: Trades in something in your pack for gold.\n\r");
	trans_line();
}

/*
 * price_it:
 *	Price the object that the hero stands on
 */
price_it()
{
	static char *bargain[] = {
		"great bargain",
		"quality product",
		"exceptional find",
	};
	reg struct linked_list *item;
	reg struct object *obj;
	reg int worth;

	if (!open_market())		/* after buying hours */
		return FALSE;
	if ((item = find_obj(hero.y,hero.x)) == NULL)
		return FALSE;
	obj = OBJPTR(item);
	if (curprice == NOTPRICED) {
		worth = get_worth(obj);
		worth += 50 - rnd(100);
		if (worth < 25)
			worth = 25;
		worth *= 3;							/* slightly expensive */
		curprice = worth;					/* save price */
		strcpy(curpurch, obj->o_typname);	/* save item */
	}
	msg("That %s is a %s for only %d pieces of gold", curpurch,
	  bargain[rnd(3)], curprice);
	return TRUE;
}

/*
 * buy_it:
 *	Buy the item on which the hero stands
 */
buy_it()
{
	reg int wh;

	if (purse <= 0) {
		msg("You have no money.");
		return;
	}
	if (curprice < 0) {		/* if not yet priced */
		wh = price_it();
		if (!wh)			/* nothing to price */
			return;
		msg("Do you want to buy it? ");
		do {
			wh = readchar();
			if (isupper(wh))
				wh = tolower(wh);
			if (wh == ESCAPE || wh == 'n') {
				msg("");
				return;
			}
		} until(wh == 'y');
	}
	mpos = 0;
	if (curprice > purse) {
		msg("You can't afford to buy that %s !",curpurch);
		return;
	}
	/*
	 * See if the hero has done all his transacting
	 */
	if (!open_market())
		return;
	/*
	 * The hero bought the item here
	 */
	mpos = 0;
	wh = add_pack(NULL,FALSE);	/* try to put it in his pack */
	if (wh) {					/* he could get it */
		purse -= curprice;		/* take his money */
		++trader;				/* another transaction */
		trans_line();			/* show remaining deals */
		curprice = NOTPRICED;
		curpurch[0] = '\0';
	}
}

/*
 * sell_it:
 *	Sell an item to the trading post
 */
sell_it()
{
	reg struct linked_list *item;
	reg struct object *obj;
	reg int wo, ch;

	if (!open_market())		/* after selling hours */
		return;

	if ((item = get_item("sell",0)) == NULL)
		return;
	obj = OBJPTR(item);
	wo = get_worth(obj);
	if (wo <= 0) {
		mpos = 0;
		msg("We don't buy those.");
		return;
	}
	if (wo < 25)
		wo = 25;
	msg("Your %s is worth %d pieces of gold.", obj->o_typname, wo);
	msg("Do you want to sell it? ");
	do {
		ch = readchar();
		if (isupper(ch))
			ch = tolower(ch);
		if (ch == ESCAPE || ch == 'n') {
			msg("");
			return;
		}
	} until (ch == 'y');
	mpos = 0;
	if (drop(item) == TRUE) {		/* drop this item */	
		nochange = FALSE;		/* show gold value */
		purse += wo;			/* give him his money */
		++trader;			/* another transaction */
		wo = obj->o_count;
		obj->o_count = 1;
		msg("Sold %s",inv_name(obj,TRUE));
		obj->o_count = wo;
		trans_line();			/* show remaining deals */
	}
}

/*
 * open_market:
 *	Retruns TRUE when ok do to transacting
 */
open_market()
{
	if (trader >= MAXPURCH) {
		msg("The market is closed. The stairs are that-a-way.");
		return FALSE;
	}
	else
		return TRUE;
}

/*
 * get_worth:
 *	Calculate an objects worth in gold
 */
get_worth(obj)
struct object *obj;
{
	reg int worth, wh;

	worth = 0;
	wh = obj->o_which;
	switch (obj->o_type) {
	case FOOD:
		worth = 2;
	when WEAPON:
		if (wh < MAXWEAPONS) {
			worth = w_magic[wh].mi_worth;
			worth *= (2 + (4 * obj->o_hplus + 4 * obj->o_dplus));
		}
	when ARMOR:
		if (wh < MAXARMORS) {
			worth = a_magic[wh].mi_worth;
			worth *= (1 + (10 * (armors[wh].a_class - obj->o_ac)));
		}
	when SCROLL:
		if (wh < MAXSCROLLS)
			worth = s_magic[wh].mi_worth;
	when POTION:
		if (wh < MAXPOTIONS)
			worth = p_magic[wh].mi_worth;
	when RING:
		if (wh < MAXRINGS) {
			worth = r_magic[wh].mi_worth;
			if (magring(obj)) {
				if (obj->o_ac > 0)
					worth += obj->o_ac * 40;
				else
					worth = 50;
			}
		}
	when STICK:
		if (wh < MAXSTICKS) {
			worth = ws_magic[wh].mi_worth;
			worth += 20 * obj->o_charges;
		}
	when AMULET:
		worth = 1000;
	otherwise:
		worth = 0;
	}
	if (worth < 0)
		worth = 0;
	if (o_on(obj, ISPROT))		/* 300% more for protected */
		worth *= 3;
	if (o_on(obj, ISBLESS))		/* 50% more for blessed */
		worth = worth * 3 / 2;
	return worth;
}

/*
 * trans_line:
 *	Show how many transactions the hero has left
 */
trans_line()
{
	sprintf(prbuf,"You have %d transactions remaining.",MAXPURCH-trader);
	mvwaddstr(cw, LINES - 4, 0, prbuf);
}

/*
 * domaze:
 *	Draw the maze on this level.
 */
do_maze()
{
	struct coord tp;
	reg int i, least;
	reg struct room *rp;
	bool treas;

	for (rp = rooms; rp < &rooms[MAXROOMS]; rp++) {
		rp->r_goldval = 0;
		rp->r_nexits = 0;			/* no exits */
		rp->r_flags = ISGONE;		/* kill all rooms */
	}
	rp = &rooms[0];					/* point to only room */
	rp->r_flags = ISDARK;			/* mazes always dark */
	rp->r_pos.x = 0;				/* room fills whole screen */
	rp->r_pos.y = 1;
	rp->r_max.x = COLS - 1;
	rp->r_max.y = LINES - 2;
	rp->r_goldval = 500 + (rnd(10) + 1) * GOLDCALC;
	draw_maze();				/* put maze into window */
	rp->r_gold = *rnd_pos(rp);
	mvaddch(rp->r_gold.y, rp->r_gold.x, GOLD);
	if (rnd(100) < 3) {			/* 3% for treasure maze level */
		treas = TRUE;
		least = 6;
		rp->r_flags |= ISTREAS;
	}
	else {						/* normal maze level */
		least = 1;
		treas = FALSE;
	}
	for (i = 0; i < level + least; i++)
		if (treas || rnd(100) < 50)		/* put in some little buggers */
			add_mon(rp, treas);
}

struct cell {
	char y_pos;
	char x_pos;
};
struct bordercells {
	char num_pos;			/* number of frontier cells next to you */
	struct cell conn[4];	/* the y,x position of above cell */
} mborder;

char *frontier, *bits;
char *moffset(), *foffset();
int tlines, tcols;

/*
 * draw_maze:
 *	Generate and draw the maze on the screen
 */
draw_maze()
{
	reg int i, j, more;
	reg char *ptr;

	tlines = (LINES - 3) / 2;
	tcols = (COLS - 1) / 2;
	bits = ALLOC((LINES - 3) * (COLS - 1));
	frontier = ALLOC(tlines * tcols);
	ptr = frontier;
	while (ptr < (frontier + (tlines * tcols)))
		*ptr++ = TRUE;
	for (i = 0; i < LINES - 3; i++) {
		for (j = 0; j < COLS - 1; j++) {
			if (i % 2 == 1 && j % 2 == 1)
				*moffset(i, j) = FALSE;		/* floor */
			else
				*moffset(i, j) = TRUE;		/* wall */
		}
	}
	for (i = 0; i < tlines; i++) {
		for (j = 0; j < tcols; j++) {
			do
				more = findcells(i,j);
			while(more != 0);
		}
	}
	crankout();
	FREE(frontier);
	FREE(bits);
}

/*
 * moffset:
 *	Calculate memory address for bits
 */
char *
moffset(y, x)
int y, x;
{
	char *ptr;

	ptr = bits + (y * (COLS - 1)) + x;
	return ptr;
}

/*
 * foffset:
 *	Calculate memory address for frontier
 */
char *
foffset(y, x)
int y, x;
{
	char *ptr;

	ptr = frontier + (y * tcols) + x;
	return ptr;
}

/*
 * findcells:
 *	Figure out cells to open up 
 */
findcells(y,x)
int x, y;
{
	reg int rtpos, i;

	*foffset(y, x) = FALSE;
	mborder.num_pos = 0;
	if (y < tlines - 1) {				/* look below */
		if (*foffset(y + 1, x)) {
			mborder.conn[mborder.num_pos].y_pos = y + 1;
			mborder.conn[mborder.num_pos].x_pos = x;
			mborder.num_pos += 1;
		}
	}
	if (y > 0) {						/* look above */
		if (*foffset(y - 1, x)) {
			mborder.conn[mborder.num_pos].y_pos = y - 1;
			mborder.conn[mborder.num_pos].x_pos = x;
			mborder.num_pos += 1;

		}
	}
	if (x < tcols - 1) {					/* look right */
		if (*foffset(y, x + 1)) {
			mborder.conn[mborder.num_pos].y_pos = y;
			mborder.conn[mborder.num_pos].x_pos = x + 1;
			mborder.num_pos += 1;
		}
	}
	if (x > 0) {						/* look left */
		if (*foffset(y, x - 1)) {
			mborder.conn[mborder.num_pos].y_pos = y;
			mborder.conn[mborder.num_pos].x_pos = x - 1;
			mborder.num_pos += 1;

		}
	}
	if (mborder.num_pos == 0)			/* no neighbors available */
		return 0;
	else {
		i = rnd(mborder.num_pos);
		rtpos = mborder.num_pos - 1;
		rmwall(mborder.conn[i].y_pos, mborder.conn[i].x_pos, y, x);
		return rtpos;
	}
}

/*
 * rmwall:
 *	Removes appropriate walls from the maze
 */
rmwall(newy, newx, oldy, oldx)
int newy, newx, oldy, oldx;
{
	reg int xdif,ydif;
	
	xdif = newx - oldx;
	ydif = newy - oldy;

	*moffset((oldy * 2) + ydif + 1, (oldx * 2) + xdif + 1) = FALSE;
	findcells(newy, newx);
}


/*
 * crankout:
 *	Does actual drawing of maze to window
 */
crankout()
{
	reg int x, y, i;

	for (y = 0; y < LINES - 3; y++) {
		move(y + 1, 0);
		for (x = 0; x < COLS - 1; x++) {
			if (*moffset(y, x)) {				/* here is a wall */
				if (y == 0 || y == LINES - 4)	/* top or bottom line */
					addch('-');
				else if (x == 0 || x == COLS - 2)	/* left | right side */
					addch('|');
				else if (y % 2 == 0 && x % 2 == 0) {
					if (*moffset(y, x - 1) || *moffset(y, x + 1))
						addch('-');
					else
						addch('|');
				}
				else if (y % 2 == 0)
					addch('-');
				else
					addch('|');
			}
			else
				addch(FLOOR);
		}
	}
}
