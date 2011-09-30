/*
 * File for the fun, ends in death or a total win
 *
 * @(#)rip.c	9.0	(rdk)	 7/17/84
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

#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#include "rogue.h"
#include "rogue.ext"

static char scoreline[100];

static char *rip[] = {
"                          ____________________",
"                         /                    \\",
"                        /  Bob Kindelberger's  \\",
"                       /       Graveyard        \\",
"                      /                          \\",
"                     /       REST IN PEACE        \\",
"                    /                              \\",
"                    |                              |",
"                    |                              |",
"                    |        Destroyed by a        |",
"                    |                              |",
"                    |                              |",
"                    |                              |",
"                    |                              |",
"                    |                              |",
"                    |                              |",
"                    |                              |",
"                   *|     *     *     *     *      |*",
"          ________)\\\\//\\//\\)/\\//\\)/\\//\\)/\\//\\)/\\//\\//(________",
};

#define RIP_LINES (sizeof rip / (sizeof (char *)))

char	*killname();

/*
 * death:
 *	Do something really fun when he dies
 */

#include <time.h>
death(monst)
char monst;
{
	reg char dp, *killer;
	struct tm *lt;
	time_t date;
	char buf[LINLEN];
	struct tm *localtime();

	time(&date);
	lt = localtime(&date);
	clear();
	move(3, 0);
	for (dp = 0; dp < RIP_LINES; dp++)
		printw("%s\n", rip[dp]);
	mvaddstr(10, 36 - ((strlen(whoami) + 1) / 2), whoami);
	killer = killname(monst);
	mvaddstr(12, 43, vowelstr(killer));
	mvaddstr(14, 36 - ((strlen(killer) + 1) / 2), killer);
	purse -= purse/10;
	sprintf(buf, "%d Gold Pieces", purse);
	mvaddstr(16, 36 - ((strlen(buf) + 1) / 2), buf);
	sprintf(prbuf, "%d/%d/%d", lt->tm_mon + 1, lt->tm_mday, 1900+lt->tm_year);
	mvaddstr(18, 32, prbuf);
	move(LINES-1, 0);
	refresh();
	score(purse, KILLED, monst);
	byebye(0);
}

/*
 * top ten entry structure
 */
static struct sc_ent {
	int sc_score;			/* gold */
	char sc_name[LINLEN];		/* players name */
	int sc_flags;			/* reason for being here */
	int sc_level;			/* dungeon level */
	int sc_uid;			/* user ID */
	unsigned char sc_monster;       /* killer */
	int sc_explvl;			/* experience level */
	long int sc_exppts;		/* experience points */
	time_t sc_date;			/* time this score was posted */
} top_ten[10];

char *reason[] = {
	"Killed",
	"Chickened out",
	"A Total Winner"
};
int oldpurse;

/*
 * score:
 *	Figure score and post it.
 */
score(amount, aflag, monst)
char monst;
int amount, aflag;
{
	reg struct sc_ent *scp, *sc2;
	reg int i, fd, prflags = 0;
	reg FILE *outf;
	char *packend;

	signal(SIGINT, byebye);
	signal(SIGQUIT, byebye);
	if (aflag != WINNER) {
		if (aflag == CHICKEN)
			packend = "when you chickened out";
		else
			packend = "at your untimely demise";
		mvaddstr(LINES - 1, 0, retstr);
		refresh();
		wgetnstr(stdscr,prbuf,80);
		oldpurse = purse;
		showpack(FALSE, packend);
	}
	/*
	 * Open file and read list
	 */
	if ((fd = open(scorefile, O_RDWR | O_CREAT, 0666)) < 0)
		return;
	outf = (FILE *) fdopen(fd, "w");
	for (scp = top_ten; scp <= &top_ten[9]; scp++) {
		scp->sc_score = 0;
		for (i = 0; i < 80; i++)
			scp->sc_name[i] = rnd(255);
		scp->sc_flags = rnd(255);
		scp->sc_level = rnd(255);
		scp->sc_monster = rnd(255);
		scp->sc_uid = rnd(255);
		scp->sc_date = rnd(255);
	}
	mvaddstr(LINES - 1, 0, retstr);
	refresh();
	wgetnstr(stdscr,prbuf,80);
	if (author() || wizard)
		if (strcmp(prbuf, "names") == 0)
			prflags = 1;
        for(i = 0; i < 10; i++)
        {
            unsigned int mon;

            encread((char *) &top_ten[i].sc_name, LINLEN, fd);
            encread((char *) scoreline, 100, fd);
            sscanf(scoreline, " %d %d %d %d %u %d %ld %lx \n",
                &top_ten[i].sc_score,   &top_ten[i].sc_flags,
                &top_ten[i].sc_level,   &top_ten[i].sc_uid,
                &mon,                   &top_ten[i].sc_explvl,
                &top_ten[i].sc_exppts,  &top_ten[i].sc_date);
            top_ten[i].sc_monster = mon;
        }
	/*
	 * Insert it in list if need be
	 */
	if (!waswizard) {
		for (scp = top_ten; scp <= &top_ten[9]; scp++)
			if (amount > scp->sc_score)
				break;
			if (scp <= &top_ten[9]) {
				for (sc2 = &top_ten[9]; sc2 > scp; sc2--)
					*sc2 = *(sc2-1);
				scp->sc_score = amount;
				strcpy(scp->sc_name, whoami);
				scp->sc_flags = aflag;
				if (aflag == WINNER)
					scp->sc_level = max_level;
				else
					scp->sc_level = level;
				scp->sc_monster = monst;
				scp->sc_uid = playuid;
				scp->sc_explvl = him->s_lvl;
				scp->sc_exppts = him->s_exp;
				time(&scp->sc_date);
		}
	}
	ignore();
	fseek(outf, 0L, 0);
        for(i = 0; i < 10; i++)
        {
            memset(scoreline,0,100);
            encwrite((char *) top_ten[i].sc_name, LINLEN, outf);
            sprintf(scoreline, " %d %d %d %d %u %d %ld %lx \n",
                top_ten[i].sc_score, top_ten[i].sc_flags,
                top_ten[i].sc_level, top_ten[i].sc_uid,
                top_ten[i].sc_monster, top_ten[i].sc_explvl,
                top_ten[i].sc_exppts, top_ten[i].sc_date);
            encwrite((char *) scoreline, 100, outf);
        }
	fclose(outf);
	signal(SIGINT, byebye);
	signal(SIGQUIT, byebye);
	clear();
	refresh();
	endwin();
	showtop(prflags);		/* print top ten list */
}

/*
 * showtop:
 *	Display the top ten on the screen
 */
showtop(showname)
int showname;
{
	reg int fd, i;
	char *killer;
	struct sc_ent *scp;

	if ((fd = open(scorefile, O_RDONLY)) < 0)
		return FALSE;
       
        for(i = 0; i < 10; i++)
        {
            unsigned int mon;
            encread((char *) &top_ten[i].sc_name, LINLEN, fd);
            encread((char *) scoreline, 100, fd);
            sscanf(scoreline, " %d %d %d %d %u %d %ld %lx \n",
                &top_ten[i].sc_score,   &top_ten[i].sc_flags,
                &top_ten[i].sc_level,   &top_ten[i].sc_uid,
                &mon,                   &top_ten[i].sc_explvl,
                &top_ten[i].sc_exppts,  &top_ten[i].sc_date);
            top_ten[i].sc_monster = mon;
        }
	close(fd);
	printf("Top Ten Adventurers:\nRank\tScore\tName\n");
	for (scp = top_ten; scp <= &top_ten[9]; scp++) {
		if (scp->sc_score > 0) {
			printf("%d\t%d\t%s: %s\t\t--> %s on level %d",
			  scp - top_ten + 1, scp->sc_score, scp->sc_name,
			  ctime(&scp->sc_date), reason[scp->sc_flags],
			  scp->sc_level);
			if (scp->sc_flags == KILLED) {
				killer = killname(scp->sc_monster);
				printf(" by a%s %s",vowelstr(killer), killer);
			}
			printf(" [Exp: %d/%ld]",scp->sc_explvl,scp->sc_exppts);
			if (showname) {
				struct passwd *pp, *getpwuid();

				if ((pp = getpwuid(scp->sc_uid)) == NULL)
					printf(" (%d)\n", scp->sc_uid);
				else
					printf(" (%s)\n", pp->pw_name);
			}
			else
				printf("\n");
		}
	}
	return TRUE;
}

/*
 * total_winner:
 *	The hero made it back out alive
 */
total_winner()
{
	clear();
addstr("                                                               \n");
addstr("  @   @               @   @           @          @@@  @     @  \n");
addstr("  @   @               @@ @@           @           @   @     @  \n");
addstr("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
addstr("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
addstr("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
addstr("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
addstr("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
addstr("                                                               \n");
addstr("     Congratulations, you have made it to the light of day!    \n");
addstr("\nYou have joined the elite ranks of those who have escaped the\n");
addstr("Dungeons of Doom alive.  You journey home and sell all your loot at\n");
addstr("a great profit and are admitted to the fighters guild.\n");

	mvaddstr(LINES - 1, 0,spacemsg);
	refresh();
	wait_for(stdscr, ' ');
	clear();
	oldpurse = purse;
	showpack(TRUE, NULL);
	score(purse, WINNER, 0);
	byebye(0);
}

/*
 * showpack:
 *	Display the contents of the hero's pack
 */
showpack(winner, howso)
bool winner;
char *howso;
{
	reg char *iname;
	reg int cnt, worth, ch;
	reg struct linked_list *item;
	reg struct object *obj;

	idenpack();
	cnt = 1;
	clear();
	if (winner)
		mvaddstr(0, 0, "   Worth  Item");
	else
		mvprintw(0, 0, "Contents of your pack %s:\n",howso);
	ch = 'a';
	for (item = pack; item != NULL; item = next(item)) {
		obj = OBJPTR(item);
		iname = inv_name(obj, FALSE);
		if (winner) {
			worth = get_worth(obj);
			worth *= obj->o_count;
			mvprintw(cnt, 0, "  %6d  %s",worth,iname);
			purse += worth;
		}
		else {
			mvprintw(cnt, 0, "%c) %s\n",ch,iname);
			ch = npch(ch);
		}
		if (++cnt >= LINES - 2 && next(item) != NULL) {
			cnt = 1;
			mvaddstr(LINES - 1, 0, morestr);
			refresh();
			wait_for(stdscr, ' ');
			clear();
		}
	}
	mvprintw(cnt + 1,0,"--- %d  Gold Pieces ---",oldpurse);
	refresh();
}

/*
 * killname:
 *	Returns what the hero was killed by.
 */
char *
killname(monst)
unsigned char monst;
{
	if (monst < MAXMONS + 1)
		return monsters[monst].m_name;
	else		/* things other than monsters */
		switch (monst) {
			case K_ARROW:	return "crooked arrow";
			case K_DART:	return "sharp dart";
			case K_BOLT:	return "jagged bolt";
			case K_POOL:	return "magic pool";
			case K_ROD:	return "exploding rod";
			case K_SCROLL:	return "burning scroll";
			case K_STONE: 	return "transmogrification to stone";
			case K_STARVE:	return "starvation";
	}
	return "Bob Kindelberger";
}
