/*
 * Rogue
 * Exploring the dungeons of doom
 *
 * @(#)main.c	9.0	(rdk)	 7/17/84
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

#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <limits.h>
#include <sys/stat.h>
#include "rogue.h"

#ifdef ATT
#include <time.h>
#endif

#ifdef BSD
#define srand48(seed)	srandom(seed)
#define lrand48()	random()
#include <sys/time.h>
#endif

#include "rogue.ext"

struct termios terminal;

main(argc, argv, envp)
char **argv;
char **envp;
{
	register char *env;
	register struct linked_list *item;
	register struct object *obj;
	struct passwd *pw;
	struct passwd *getpwuid();
	char alldone, wpt;
	char *getpass(), *xcrypt(), *strrchr();
	int lowtime;
	time_t now;
        char *roguehome();
	char *homedir = roguehome();

#ifdef __DJGPP__
	_fmode = O_BINARY;
#endif

	if (homedir == NULL)
        	homedir = "";

	playuid = getuid();

	if (setuid(playuid) < 0) {
		printf("Cannot change to effective uid: %d\n", playuid);
		exit(1);
	}
	playgid = getgid();

	/* check for print-score option */

	strcpy(scorefile, homedir);

	if (*scorefile)
		strcat(scorefile,"/");
	strcat(scorefile, "srogue.scr");

	if(argc >= 2 && strcmp(argv[1], "-s") == 0)
	{
		showtop(0);
		exit(0);
	}

	if (argc >= 2 && author() && strcmp(argv[1],"-a") == 0)
	{
		wizard = TRUE;
		argv++;
		argc--;
	}

	/* Check to see if he is a wizard */

	if (argc >= 2 && strcmp(argv[1],"-w") == 0)
	{
		if (strcmp(PASSWD, xcrypt(getpass(wizstr),"mT")) == 0)
		{
			wizard = TRUE;
			argv++;
			argc--;
		}
	}
	time(&now);
	lowtime = (int) now;

	/* get home and options from environment */

	if ((env = getenv("HOME")) != NULL)
		strcpy(home, env);
	else if ((pw = getpwuid(playuid)) != NULL)
		strcpy(home, pw->pw_dir);
	else
		home[0] = '\0';

        if (strcmp(home,"/") == 0)
		home[0] = '\0';

        if ((strlen(home) > 0) && (home[strlen(home)-1] != '/'))
		strcat(home, "/");

	strcpy(file_name, home);
	strcat(file_name, "srogue.sav");

	if ((env = getenv("ROGUEOPTS")) != NULL)
		parse_opts(env);

	if (env == NULL || whoami[0] == '\0')
	{
		if((pw = getpwuid(playuid)) == NULL)
		{
			printf("Say, who are you?\n");
			exit(1);
		}
		else
			strucpy(whoami, pw->pw_name, strlen(pw->pw_name));
	}

	if (env == NULL || fruit[0] == '\0')
		strcpy(fruit, "juicy-fruit");

	if (argc == 2)
		if(!restore(argv[1], envp)) /* NOTE: NEVER RETURNS */
			exit(1);

	dnum = (wizard && getenv("SEED") != NULL ?
		atoi(getenv("SEED")) : lowtime + getpid());

	if(wizard)
		printf("Hello %s, welcome to dungeon #%d\n", whoami, dnum);
	else
		printf("Hello %s, One moment while I open the door to the dungeon...\n", whoami);

	fflush(stdout);
	seed = dnum;
	srand48(seed);			/* init rnd number gen */

	signal(SIGINT, byebye);		/* just in case */
	signal(SIGQUIT ,byebye);

	init_everything();

#ifdef __INTERIX
        setenv("TERM","interix");
#endif

	initscr();			/* Start up cursor package */

	if (strcmp(termname(),"dumb") == 0)
	{
		endwin();
		printf("ERROR in terminal parameters.\n");
		printf("Check TERM in environment.\n");
		byebye(1);
	}

	if (LINES < 24 || COLS < 80) {
		endwin();
		printf("ERROR: screen size too small\n");
		byebye(1);
	}

	if ((whoami == NULL) || (*whoami == '\0') || (strcmp(whoami,"dosuser")==0))
	{
		echo();
		mvaddstr(23,2,"Rogue's Name? ");
		wgetnstr(stdscr,whoami,MAXSTR);
		noecho();
	}

	if ((whoami == NULL) || (*whoami == '\0'))
		strcpy(whoami,"Rodney");
	
	setup();

	/* Set up windows */

	cw = newwin(0, 0, 0, 0);
	mw = newwin(0, 0, 0, 0);
	hw = newwin(0, 0, 0, 0);
	waswizard = wizard;

	/* Draw current level */

	new_level(NORMLEV);

	/* Start up daemons and fuses */

	daemon(status, TRUE, BEFORE);
	daemon(doctor, TRUE, BEFORE);
	daemon(stomach, TRUE, BEFORE);
	daemon(runners, TRUE, AFTER);
	fuse(swander, TRUE, WANDERTIME);

	/* Give the rogue his weaponry */

	do {
		wpt = pick_one(w_magic);
		switch (wpt)
		{
			case MACE:	case SWORD:	case TWOSWORD:
			case SPEAR:	case TRIDENT:	case SPETUM:
			case BARDICHE:	case PIKE:	case BASWORD:
			case HALBERD:
				alldone = TRUE;
			otherwise:
				alldone = FALSE;
		}
	} while(!alldone);

	item = new_thing(FALSE, WEAPON, wpt);
	obj = OBJPTR(item);
	obj->o_hplus = rnd(3);
	obj->o_dplus = rnd(3);
	obj->o_flags = ISKNOW;
	add_pack(item, TRUE);
	cur_weapon = obj;

	/* Now a bow */

	item = new_thing(FALSE, WEAPON, BOW);
	obj = OBJPTR(item);
	obj->o_hplus = rnd(3);
	obj->o_dplus = rnd(3);
	obj->o_flags = ISKNOW;
	add_pack(item, TRUE);

	/* Now some arrows */

	item = new_thing(FALSE, WEAPON, ARROW);
	obj = OBJPTR(item);
	obj->o_count = 25 + rnd(15);
	obj->o_hplus = rnd(2);
	obj->o_dplus = rnd(2);
	obj->o_flags = ISKNOW;
	add_pack(item, TRUE);

	/* And his suit of armor */

	wpt = pick_one(a_magic);
	item = new_thing(FALSE, ARMOR, wpt);
	obj = OBJPTR(item);
	obj->o_flags = ISKNOW;
	obj->o_ac = armors[wpt].a_class - rnd(4);
	cur_armor = obj;
	add_pack(item, TRUE);
	
	/* Give him some food */

	item = new_thing(FALSE, FOOD, 0);
	add_pack(item, TRUE);

	playit();
}


/*
 * endit:
 *	Exit the program abnormally.
 */
void
endit(int a)
{
	fatal("Ok, if you want to exit that badly, I'll have to allow it");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */

fatal(s)
char *s;
{
	clear();
	refresh();
	endwin();
	fprintf(stderr,"%s\n\r",s);
	fflush(stderr);
	byebye(2);
}

/*
 * byebye:
 *	Exit here and reset the users terminal parameters
 *	to the way they were when he started
 */

void
byebye(how)
int how;
{
	if (!isendwin())
		endwin();

	exit(how);		/* exit like flag says */
}


/*
 * rnd:
 *	Pick a very random number.
 */
rnd(range)
int range;
{
	reg int wh;

	if (range == 0)
		wh = 0;
	else {
		wh = lrand48() % range;
		wh &= 0x7FFFFFFF;
	}
	return wh;
}

/*
 * roll:
 *	roll a number of dice
 */
roll(number, sides)
int number, sides;
{
	reg int dtotal = 0;

	while(number-- > 0)
		dtotal += rnd(sides)+1;
	return dtotal;
}


/*
** setup: 	Setup signal catching functions
*/
setup()
{
	signal(SIGHUP, auto_save);
	signal(SIGINT, auto_save);
	signal(SIGQUIT,  byebye);
	signal(SIGILL, game_err);
	signal(SIGTRAP, game_err);
#ifdef SIGIOT
	signal(SIGIOT, game_err);
#endif
#ifdef SIGEMT
	signal(SIGEMT, game_err);
#endif
	signal(SIGFPE, game_err);
#ifdef SIGBUS
	signal(SIGBUS, game_err);
#endif
	signal(SIGSEGV, game_err);
#ifdef SIGSYS
	signal(SIGSYS, game_err);
#endif
	signal(SIGPIPE, game_err);
	signal(SIGTERM, game_err);

	cbreak();
	noecho();
}

/*
** playit:	The main loop of the program.  Loop until the game is over,
**		refreshing things and looking at the proper times.
*/

playit()
{
	reg char *opts;

	tcgetattr(0,&terminal);


	/* parse environment declaration of options */

	if ((opts = getenv("ROGUEOPTS")) != NULL)
		parse_opts(opts);

	player.t_oldpos = hero;
	oldrp = roomin(&hero);
	nochange = FALSE;
	while (playing)
		command();		/* Command execution */
	endit(0);
}


/*
** author:	See if a user is an author of the program
*/
author()
{
	switch (playuid) {
		case 100:
		case 0:
			return TRUE;
		default:
			return FALSE;
	}
}

int
directory_exists(char *dirname)
{
    struct stat sb;

    if (stat(dirname, &sb) == 0) /* path exists */
        return (S_ISDIR (sb.st_mode));

    return(0);
}

char *
roguehome()
{
    static char path[1024];
    char *end,*home;

    if ( (home = getenv("ROGUEHOME")) != NULL)
    {
        if (*home)
        {
            strncpy(path, home, PATH_MAX - 20);

            end = &path[strlen(path)-1];


            while( (end >= path) && ((*end == '/') || (*end == '\\')))
                *end-- = '\0';

            if (directory_exists(path))
                return(path);
        }
    }

    if (directory_exists("/var/games/roguelike"))
        return("/var/games/roguelike");
    if (directory_exists("/var/lib/roguelike"))
        return("/var/lib/roguelike");
    if (directory_exists("/var/roguelike"))
        return("/var/roguelike");
    if (directory_exists("/usr/games/lib"))
        return("/usr/games/lib");
    if (directory_exists("/games/roguelik"))
        return("/games/roguelik");

    return(NULL);
}

