/*
 * This file has all the code for the option command.
 *
 * @(#)options.c	9.0	(rdk)	 7/17/84
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

#include <termios.h>
#include <ctype.h>
#include "rogue.h"
#include "rogue.ext"

extern struct termios terminal;

/*
 * description of an option and what to do with it
 */
struct optstruct {
	char	*o_name;	/* option name */
	char	*o_prompt;	/* prompt for interactive entry */
	char	*o_opt;		/* pointer to thing to set */
};

typedef struct optstruct	OPTION;

int	put_str(), get_str();

OPTION	optlist[] = {
	{ "name",	"Name: ",		whoami },
	{ "fruit",	"Fruit: ",		fruit },
	{ "file", 	"Save file: ",	file_name }
};
#define	NUM_OPTS	(sizeof optlist / sizeof (OPTION))

/*
 * print and then set options from the terminal
 */
option()
{
	reg OPTION	*op;
	reg int	wh;

	wclear(hw);
	touchwin(hw);
	/*
	 * Display current values of options
	 */
	for (op = optlist; op < &optlist[NUM_OPTS]; op++) {
		wh = op - optlist;
		mvwaddstr(hw, wh, 0, op->o_prompt);
		mvwaddstr(hw, wh, 16, op->o_opt);
	}
	/*
	 * Set values
	 */
	wmove(hw, 0, 0);
	for (op = optlist; op < &optlist[NUM_OPTS]; op++) {
		wmove(hw, op - optlist, 16);
		if ((wh = get_str(op->o_opt, hw))) {
			if (wh == QUIT)
				break;
			else if (op > optlist) {
				wmove(hw, op - optlist, 0);
				op -= 2;
			}
			else {
				putchar(7);
				wmove(hw, 0, 0);
				op -= 1;
			}
		}
	}
	/*
	 * Switch back to original screen
	 */
	dbotline(hw,spacemsg);
	restscr(cw);
	after = FALSE;
}


/*
 * get_str:
 *	Set a string option
 */
#define CTRLB	2
get_str(opt, awin)
char *opt;
WINDOW *awin;
{
	reg char *sp;
	reg int c, oy, ox;
	char buf[LINLEN];

	draw(awin);
	getyx(awin, oy, ox);
	/*
	 * loop reading in the string, and put it in a temporary buffer
	 */
	for (sp = buf; (c=wgetch(awin)) != '\n' && c != '\r' && c != ESCAPE;
	  wclrtoeol(awin), draw(awin)) {
		if (( (int)sp - (int)buf ) >= 50) {
			*sp = '\0';			/* line was too long */
			strucpy(opt,buf,strlen(buf));
			mvwaddstr(awin, 0, 0, "Name was truncated --More--");
			wclrtoeol(awin);
			draw(awin);
			wait_for(awin, ' ');
			mvwprintw(awin, 0, 0, "Called: %s",opt);
			draw(awin);
			return NORM;
		}
		if (c == -1)
			continue;
		else if(c == terminal.c_cc[VERASE])	{	/* process erase char */
			if (sp > buf) {
				reg int i;
	
				sp--;
				for (i = strlen(unctrl(*sp)); i; i--)
					waddch(awin, '\b');
			}
			continue;
		}
		else if (c == terminal.c_cc[VKILL]) {   /* process kill character */
			sp = buf;
			wmove(awin, oy, ox);
			continue;
		}
		else if (sp == buf) {
			if (c == CTRLB)			/* CTRL - B */
				break;
			if (c == '~') {
				strcpy(buf, home);
				waddstr(awin, home);
				sp += strlen(home);
				continue;
			}
		}
		*sp++ = c;
		waddstr(awin, unctrl(c));
	}
	*sp = '\0';
	if (sp > buf)	/* only change option if something was typed */
		strucpy(opt, buf, strlen(buf));
	wmove(awin, oy, ox);
	waddstr(awin, opt);
	waddstr(awin, "\n\r");
	draw(awin);
	if (awin == cw)
		mpos += sp - buf;
	if (c == CTRLB)
		return MINUS;
	if (c == ESCAPE)
		return QUIT;
	return NORM;
}

/*
 * parse_opts:
 *	Parse options from string, usually taken from the environment.
 *	the string is a series of comma seperated values, with strings
 *	being "name=....", with the string being defined up to a comma
 *	or the end of the entire option string.
 */

parse_opts(str)
char *str;
{
	reg char *sp;
	reg OPTION *op;
	reg int len;

	while (*str) {
		for (sp = str; isalpha(*sp); sp++)	/* get option name */
			continue;
		len = sp - str;
		for (op = optlist; op < &optlist[NUM_OPTS]; op++) {
			if (EQSTR(str, op->o_name, len)) {
				reg char *start;
	
				for (str = sp + 1; *str == '='; str++)
					continue;
				if (*str == '~') {
					strcpy(op->o_opt, home);
					start = op->o_opt + strlen(home);
					while (*++str == '/')
						continue;
				}
				else
					start = (char *) op->o_opt;
				for (sp = str + 1; *sp && *sp != ','; sp++)
					continue;
				strucpy(start, str, sp - str);
			}
		}
		/*
		 * skip to start of next option name
		 */
		while (*sp && !isalpha(*sp))
			sp++;
		str = sp;
	}
}

/*
 * copy string using unctrl for things
 */
strucpy(s1, s2, len)
char *s1, *s2;
int len;
{
	reg char *sp;

	while (len-- > 0) {
		strcpy(s1, (sp = unctrl(*s2)));
		s1 += strlen(sp);
		s2++;
	}
	*s1 = '\0';
}
