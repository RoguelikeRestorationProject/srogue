/*
 * Various input/output functions
 *
 * @(#)io.c	9.0	(rdk)	 7/17/84
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

#include <stdarg.h>
#include <ctype.h>
#include "rogue.h"
#include "rogue.ext"

/*
 * msg:
 *	Display a message at the top of the screen.
 */
static char msgbuf[BUFSIZ];
static int newpos = 0;

msg(char *fmt, ...)
{
	va_list ap;
	/*
	 * if the string is "", just clear the line
	 */
	if (*fmt == '\0') {
		wmove(cw, 0, 0);
		wclrtoeol(cw);
		mpos = 0;
		return;
	}
	/*
	 * otherwise add to the message and flush it out
	 */
	va_start(ap, fmt);
	doadd(fmt, ap);
	va_end(ap);
	endmsg();
}

/*
 * addmsg:
 *	Add things to the current message
 */
addmsg(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	doadd(fmt, ap);
	va_end(ap);
}

/*
 * endmsg:
 * 	Display a new msg, giving him a chance to see the
 *	previous one if it is up there with the --More--
 */
endmsg()
{
	strcpy(huh, msgbuf);
	if (mpos > 0) {
		wmove(cw, 0, mpos);
		waddstr(cw, morestr);
		draw(cw);
		wait_for(cw, ' ');
	}
	mvwaddstr(cw, 0, 0, msgbuf);
	wclrtoeol(cw);
	mpos = newpos;
	newpos = 0;
	draw(cw);
}

/*
 * doadd:
 *	Perform a printf into a buffer
 */
doadd(char *fmt, va_list ap)
{
	vsprintf(&msgbuf[newpos], fmt, ap);
	newpos = strlen(msgbuf);
}

/*
 * step_ok:
 *	Returns TRUE if it is ok to step on ch
 */
step_ok(ch)
unsigned char ch;
{
	if (dead_end(ch))
		return FALSE;
	else if (ch >= 32 && ch <= 127 && !isalpha(ch))
		return TRUE;
	return FALSE;
}


/*
 * dead_end:
 *	Returns TRUE if you cant walk through that character
 */
dead_end(ch)
char ch;
{
	if (ch == '-' || ch == '|' || ch == ' ' || ch == SECRETDOOR)
		return TRUE;
	else
		return FALSE;
}


/*
 * readchar:
 *	flushes stdout so that screen is up to date and then returns
 *	getchar.
 */

readchar()
{
	char c;

	fflush(stdout);
        return( wgetch(cw) );
}

char *hungstr[] = {
	"",
	"  HUNGRY",
	"  STARVING",
	"  FAINTING",
};

/*
 * status:
 *	Display the important stats line.  Keep the cursor where it was.
 */
status(fromfuse)
int fromfuse;
{
	reg int totwght, carwght;
	reg struct real *stef, *stre, *stmx;
	reg char *pb;
	int oy, ox, ch;
	static char buf[LINLEN];
	static char hwidth[] = { "%2d(%2d)" };

	/*
	 * If nothing has changed since the last time, then done
	 */
	if (nochange)
		return;
	nochange = TRUE;
	updpack();					/* get all weight info */
	stef = &player.t_stats.s_ef;
	stre = &player.t_stats.s_re;
	stmx = &max_stats.s_re;
	totwght = him->s_carry / 10;
	carwght = him->s_pack / 10;
	getyx(cw, oy, ox);
	if (him->s_maxhp >= 100) {
		hwidth[1] = '3';	/* if hit point >= 100	*/
		hwidth[5] = '3';	/* change %2d to %3d	*/
	}
	if (stre->a_str < stmx->a_str)
		ch = '*';
	else
		ch = ' ';
	sprintf(buf, "Str: %2d(%c%2d)", stef->a_str, ch, stre->a_str);
	pb = &buf[strlen(buf)];
	if (stre->a_dex < stmx->a_dex)
		ch = '*';
	else
		ch = ' ';
	sprintf(pb, "  Dex: %2d(%c%2d)", stef->a_dex, ch, stre->a_dex);
	pb = &buf[strlen(buf)];
	if (stre->a_wis < stmx->a_wis)
		ch = '*';
	else
		ch = ' ';
	sprintf(pb, "  Wis: %2d(%c%2d)", stef->a_wis, ch, stre->a_wis);
	pb = &buf[strlen(buf)];
	if (stre->a_con < stmx->a_con)
		ch = '*';
	else
		ch = ' ';
	sprintf(pb, "  Con: %2d(%c%2d)", stef->a_con, ch, stre->a_con);
	pb = &buf[strlen(buf)];
	sprintf(pb, "  Carry: %3d(%3d)", carwght, totwght);
	mvwaddstr(cw, LINES - 1, 0, buf);
	sprintf(buf, "Level: %d  Gold: %5d  Hp: ",level, purse);
	pb = &buf[strlen(buf)];
	sprintf(pb, hwidth, him->s_hpt, him->s_maxhp);
	pb = &buf[strlen(buf)];
	sprintf(pb,"  Ac: %-2d  Exp: %d/%ld",cur_armor == NULL ? him->s_arm :
	  cur_armor->o_ac, him->s_lvl, him->s_exp);
	carwght = (packvol * 100) / V_PACK;
	pb = &buf[strlen(buf)];
	sprintf(pb, "  Vol: %3d%%", carwght);
	mvwaddstr(cw, LINES - 2, 0, buf);
	waddstr(cw, hungstr[hungry_state]);
	wclrtoeol(cw);
	wmove(cw, oy, ox);
}

/*
 * dispmax:
 *	Display the hero's maximum status
 */
dispmax()
{
	reg struct real *hmax;

	hmax = &max_stats.s_re;
	msg("Maximums:  Str = %d  Dex = %d  Wis = %d  Con = %d",
		hmax->a_str, hmax->a_dex, hmax->a_wis, hmax->a_con);
}

/*
 * illeg_ch:
 * 	Returns TRUE if a char shouldn't show on the screen
 */
illeg_ch(ch)
unsigned char ch;
{
	if (ch < 32 || ch > 127)
		return TRUE;
	if (ch >= '0' && ch <= '9')
		return TRUE;
	return FALSE;
}

/*
 * wait_for:
 *	Sit around until the guy types the right key
 */
wait_for(win,ch)
WINDOW *win;
char ch;
{
	register char c;

	if (ch == '\n')
	    while ((c = wgetch(win)) != '\n' && c != '\r')
			continue;
	else
	    while (wgetch(win) != ch)
			continue;
}

#ifdef NEED_GETTIME
#include <stdio.h>
#include <pwd.h>

/*
 * gettime:
 *	This routine returns the current time as a string
 */
#ifdef ATT
#include <time.h>
#endif
#ifdef BSD
#include <sys/time.h>
#endif

char *
gettime()
{
	register char *timeptr;
	char *ctime();
	long int now, time();

	time(&now);		/* get current time */
	timeptr = ctime(&now);	/* convert to string */
	return timeptr;		/* return the string */
}
#endif


/*
 * dbotline:
 *	Displays message on bottom line and waits for a space to return
 */
dbotline(scr,message)
WINDOW *scr;
char *message;
{
	mvwaddstr(scr,LINES-1,0,message);
	draw(scr);
	wait_for(scr,' ');	
}


/*
 * restscr:
 *	Restores the screen to the terminal
 */
restscr(scr)
WINDOW *scr;
{
	clearok(scr,TRUE);
	touchwin(scr);
}

/*
 * npch:
 *	Get the next char in line for inventories
 */
npch(ch)
char ch;
{
	reg char nch;
	if (ch >= 'z')
		nch = 'A';
	else
		nch = ch + 1;
	return nch;
}
