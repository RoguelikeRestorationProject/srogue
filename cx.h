/*
 * Super-Rogue
 * Copyright (C) 1984 Robert D. Kindelberger
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include	<stdio.h>
#include	<sgtty.h>

#define	bool	char		/* boolean variable	*/
#define	reg	register	/* register abbr.	*/

#define	TRUE	(1)
#define	FALSE	(0)
#define	ERR	(0)	/* default return on error	*/
#define	OK	(1)	/* default return on good run	*/

#define	_SUBWIN	01	/* window is a subwindow	*/
#define	_ENDLINE 02	/* lines go to end of screen	*/
#define	_FULLWIN 04	/* window is entire screen	*/
#define	_SCROLLWIN 010	/* window could cause scroll	*/
#define	_STANDOUT 0200	/* standout mode in effect	*/
#define	_NOCHANGE -1	/* no change on this line	*/

#define	_puts(s)	tputs(s, 0, _putchar);

typedef	struct sgttyb	SGTTY;

#ifndef WINDOW

#define	WINDOW	struct _win_st

/* window description structure		*/
struct _win_st {
    short _cury, _curx;		/* current y,x positions */
    short _maxy, _maxx;		/* maximum y,x positions */
    short _begy, _begx;		/* start y,x positions	 */
    short _flags;		/* various window flags	 */
    bool _clear;		/* need to clear	 */
    bool _leave;		/* leave cur x,y at last update	*/
    bool _scroll;		/* scrolls allowed	*/
    char **_y;			/* actual window	*/
    short *_firstch;		/* first change on line	*/
    short *_lastch;		/* last change on line	*/
};

extern bool My_term,	/* user specied terminal	*/
	    _echoit,	/* set if echoing characters	*/
	    _rawmode;	/* set if terminal in raw mode	*/

extern char *Def_term,	/* default terminal type	*/
	    ttytype[];	/* long name of current term	*/
# ifdef DEBUG
extern FILE *outf;	/* error outfile		*/
# endif

extern int LINES, COLS;	/* # of lines & columns		*/
extern int _tty_ch;	/* channel with tty on it	*/
extern WINDOW *stdscr, *curscr;

static SGTTY _tty, _res_flg;

/*
 * Define VOID to stop lint from generating "null effect"
 * comments.
 */
# ifdef lint
int	__void__;	/* place to assign to		*/

# define	VOID(x)	(__void__ = (int) (x))
# else
# define	VOID(x)	(x)
# endif

# endif

/*
 * psuedo functions for standard screen
 */
#define	addch(ch)	VOID(waddch(stdscr, ch))
#define	getch()		VOID(wgetch(stdscr))
#define	addstr(str)	VOID(waddstr(stdscr, str))
#define	getstr(str)	VOID(wgetstr(stdscr, str))
#define	move(y, x)	VOID(wmove(stdscr, y, x))
#define	clear()		VOID(wclear(stdscr))
#define	erase()		VOID(werase(stdscr))
#define	clrtobot()	VOID(wclrtobot(stdscr))
#define	clrtoeol()	VOID(wclrtoeol(stdscr))
#define	insertln()	VOID(winsertln(stdscr))
#define	deleteln()	VOID(wdeleteln(stdscr))
#define	refresh()	VOID(wrefresh(stdscr))
#define	inch()		VOID(winch(stdscr))

#ifdef STANDOUT
#define	standout()	VOID(wstandout(stdscr))
#define	standend()	VOID(wstandend(stdscr))
#endif

/*
# define	CBREAK 	FALSE
# define	_IOSTRG 01
*/

/*
 * mv functions
 */
#define	mvwaddch(win,y,x,ch)	VOID(wmove(win,y,x)==ERR?ERR:waddch(win,ch))
#define	mvwgetch(win,y,x,ch)	VOID(wmove(win,y,x)==ERR?ERR:wgetch(win,ch))
#define	mvwaddstr(win,y,x,str)	VOID(wmove(win,y,x)==ERR?ERR:waddstr(win,str))
#define	mvwgetstr(win,y,x,str)	VOID(wmove(win,y,x)==ERR?ERR:wgetstr(win,str))
#define	mvwinch(win,y,x)	VOID(wmove(win,y,x) == ERR ? ERR : winch(win))
#define	mvaddch(y,x,ch)		mvwaddch(stdscr,y,x,ch)
#define	mvgetch(y,x,ch)		mvwgetch(stdscr,y,x,ch)
#define	mvaddstr(y,x,str)	mvwaddstr(stdscr,y,x,str)
#define	mvgetstr(y,x,str)	mvwgetstr(stdscr,y,x,str)
#define	mvinch(y,x)		mvwinch(stdscr,y,x)

/*
 * psuedo functions
 */

#define	clearok(win,bf)	 (win->_clear = bf)
#define	leaveok(win,bf)	 (win->_leave = bf)
#define	scrollok(win,bf) (win->_scroll = bf)
#define	getyx(win,y,x)	 y = win->_cury, x = win->_curx
#define	winch(win)	 (win->_y[win->_cury][win->_curx])

WINDOW	*initscr(), *newwin();
