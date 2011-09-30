/*
 * save and restore routines
 *
 * @(#)save.c	9.0	(rdk)	 7/17/84
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

#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include "rogue.h"
#include "rogue.ext"

#ifdef BSD
#define srand48(seed)	srandom(seed)
#endif

EXTCHAR version[];
EXTCHAR *ctime();

typedef struct stat STAT;
STAT sbuf;

/*
 * ignore:
 *	Ignore ALL signals possible
 */
ignore()
{
	int i;

	for (i = 0; i < NSIG; i++)
		signal(i, SIG_IGN);
}

/*
 * save_game:
 *	Save the current game
 */
save_game()
{
	reg FILE *savef;
	reg int c;
	char buf[LINLEN];

	mpos = 0;
	if (file_name[0] != '\0') {
		msg("Save file (%s)? ", file_name);
		do {
			c = wgetch(cw);
			if(c == ESCAPE) {
				msg("");
				return FALSE;
			}
		} while (c != 'n' && c != 'y');
		mpos = 0;
		if (c == 'y')
			goto gotfile;
	}
	msg("File name: ");
	mpos = 0;
	buf[0] = '\0';
	if (get_str(buf, cw) == QUIT) {
		msg("");
		return FALSE;
	}
	msg("");
	strcpy(file_name, buf);
gotfile:
	c = dosave();		/* try to save this game */
	if (c == FALSE)
		msg("Could not save game to file %s", file_name);
	return c;
}

/*
 * auto_save:
 *	Automatically save a game
 */
void
auto_save(int a)
{
	dosave();		/* save this game */
	byebye(1);		/* so long for now */
}

/*
 * game_err:
 *	When an error occurs. Set error flag and save game.
 */
void
game_err(int a)
{
	int ok;

	ok = dosave();			/* try to save this game */
	clear();
	refresh();
	endwin();

	printf("\nInternal error !!!\n\nYour game was ");
	if (ok)
		printf("saved.");
	else
		printf("NOT saveable.");

	fflush(stdout);

#ifdef SIGIOT
	signal(SIGIOT, SIG_DFL);	/* allow core dump signal */
#endif

	abort();			/* cause core dump */
	byebye(3);
}

/*
 * dosave:
 *	Set UID back to user and save the game
 */
dosave()
{
	FILE *savef;

	ignore();
	setuid(playuid);
	setgid(playgid);
	umask(022);

	if (file_name[0] != '\0') {
		if ((savef = fopen(file_name,"w")) != NULL)
		{
			save_file(savef);
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * save_file:
 *	Do the actual save of this game to a file
 */
save_file(savef)
FILE *savef;
{
	reg int fnum;
	int slines = LINES;
	int scols = COLS;

#ifdef __DJGPP__                      /* st_ino w/ DJGPP under WinXP broken */
        _djstat_flags |= _STAT_INODE; /* so turn off computing it for now   */
#endif

	/*
	 * force allocation of the buffer now so that inodes, etc
	 * can be checked when restoring saved games.
	 */
	fnum = fileno(savef);
	fstat(fnum, &sbuf);
	write(fnum, "RDK", 4);
	lseek(fnum, 0L, 0);
	encwrite(version,strlen(version)+1,savef);
	encwrite(&sbuf.st_ino,sizeof(sbuf.st_ino),savef);
	encwrite(&sbuf.st_dev,sizeof(sbuf.st_dev),savef);
	encwrite(&sbuf.st_ctime,sizeof(sbuf.st_ctime),savef);
	encwrite(&sbuf.st_mtime,sizeof(sbuf.st_mtime),savef);
	encwrite(&slines,sizeof(slines),savef);
	encwrite(&scols,sizeof(scols),savef);
	msg("");
	rs_save_file(savef);
	close(fnum);
	signal(SIGINT, byebye);
	signal(SIGQUIT, byebye);
	wclear(cw);
	draw(cw);
}

/*
 * restore:
 *	Restore a saved game from a file
 */
restore(file, envp)
char *file, **envp;
{
	register inf, pid;
	int ret_status;
#ifndef _AIX
	extern char **environ;
#endif
#ifdef __DJGPP__                      /* st_ino w/ DJGPP under WinXP broken */
        _djstat_flags |= _STAT_INODE; /* so turn off computing it for now   */
#endif
	char buf[LINLEN];
	STAT sbuf2;
	int slines, scols;

	if ((inf = open(file, O_RDONLY)) < 0) {
		printf("Cannot read save game %s\n",file);
		return FALSE;
	}

	encread(buf, strlen(version) + 1, inf);

	if (strcmp(buf, version) != 0) {
		printf("Sorry, saved game version is out of date.\n");
		return FALSE;
	}

	fstat(inf, &sbuf2);

	encread(&sbuf.st_ino,sizeof(sbuf.st_ino), inf);
	encread(&sbuf.st_dev,sizeof(sbuf.st_dev), inf);
	encread(&sbuf.st_ctime,sizeof(sbuf.st_ctime), inf);
	encread(&sbuf.st_mtime,sizeof(sbuf.st_mtime), inf);
	encread(&slines,sizeof(slines),inf);
	encread(&scols,sizeof(scols),inf);

	/*
	 * we do not close the file so that we will have a hold of the
	 * inode for as long as possible
	 */

	if (!wizard)
	{
		if(sbuf2.st_ino!=sbuf.st_ino || sbuf2.st_dev!=sbuf.st_dev) {
			printf("Sorry, saved game is not in the same file.\n");
			return FALSE;
		}
	}

#ifdef __INTERIX
	setenv("TERM","interix");
#endif

	initscr();

	if (slines > LINES)
	{
		endwin();
		printf("Sorry, original game was played on a screen with %d lines.\n",slines);
		printf("Current screen only has %d lines. Unable to restore game\n",LINES);
		return(FALSE);
	}

	if (scols > COLS)
	{
		endwin();
		printf("Sorry, original game was played on a screen with %d columns.\n", scols);
		printf("Current screen only has %d columns. Unable to restore game\n",COLS);
		return(FALSE);
	}

	cw = newwin(LINES, COLS, 0, 0);
	mw = newwin(LINES, COLS, 0, 0);
	hw = newwin(LINES, COLS, 0, 0);

	mpos = 0;
	mvwprintw(cw, 0, 0, "%s: %s", file, ctime(&sbuf2.st_mtime));

	/* defeat multiple restarting from the same place */

	if (!wizard)
	{
		if (sbuf2.st_nlink != 1)
		{
                        endwin();
			printf("Cannot restore from a linked file\n");
			return FALSE;
		}
	}

	if (rs_restore_file(inf) == FALSE)
	{
		endwin();
		printf("Cannot restore file\n");
		return(FALSE);
	}

#if defined(__CYGWIN__) || defined(__DJGPP__)
	close(inf);
#endif
	if (!wizard)
	{
#ifndef __DJGPP__
			endwin();
			while((pid = fork()) < 0)
				sleep(1);

			/* set id to unlink file */
			if(pid == 0)
			{
				setuid(playuid);
				setgid(playgid);
				unlink(file);
				exit(0);
			}
			/* wait for unlink to finish */
			else
			{
				while(wait(&ret_status) != pid)
					continue;
				if (ret_status < 0)
				{
					printf("Cannot unlink file\n");
					return FALSE;
				}
			}
#else
		if (unlink(file) < 0)
		{
			printf("Cannot unlink file\n");
			return FALSE;
		}
#endif

	}

	environ = envp;

	strcpy(file_name, file);
	setup();
	restscr(cw);
	srand48(getpid());
	playit();
}
