/*
 * Super-Rogue
 * Copyright (C) 1984 Robert D. Kindelberger
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "rogue.h"

extern SGTTY _tty, _res_flg;
extern bool NONL;

raw()
{
/*
	VERSION 5.0
	_tty.c_lflag &= ~ICANON;
	_tty.c_cc[VMIN] = 1;
	_tty.c_cc[VTIME] = 255;
	_tty.c_oflag &= ~OPOST;
*/
	_rawmode = TRUE;
	_tty.sg_flags |= CBREAK;
	ioctl(_tty_ch, TIOCSETP, &_tty);
}


noraw()
{
/*
	VERSION 5.0
	_tty.c_lflag |= ICANON;
	_tty.c_cc[VMIN] = _res_flg.c_cc[VMIN];
	_tty.c_cc[VTIME] = _res_flg.c_cc[VTIME];
	_tty.c_oflag |= OPOST;
*/
	_rawmode = FALSE;
	_tty.sg_flags &= ~CBREAK;
	ioctl(_tty_ch, TIOCSETP, &_tty);
}


crmode()
{
/*
	VERSION 5.0
	_tty.c_lflag &= ~ICANON;
	_tty.c_oflag |= ONLCR;
	_tty.c_cc[VMIN] = 1;
	_tty.c_cc[VTIME]=255;
*/
	_rawmode = TRUE;
	_tty.sg_flags |= (CBREAK | CRMOD);
	ioctl(_tty_ch, TIOCSETP, &_tty);
}


nocrmode()
{
/*
	_tty.c_lflag |= ICANON;
	_tty.c_cc[VMIN]=_res_flg.c_cc[VMIN];
	_tty.c_cc[VTIME]=_res_flg.c_cc[VTIME];
*/
	_rawmode = FALSE;
	_tty.sg_flags &= ~CBREAK;
	ioctl(_tty_ch, TIOCSETP, &_tty);
}


echo()
{
	_tty.sg_flags |= ECHO;
	_echoit=TRUE;
	ioctl(_tty_ch, TIOCSETP, &_tty);
}

noecho()
{
	_tty.sg_flags &= ~ECHO;
	_echoit = FALSE;
	ioctl(_tty_ch, TIOCSETP, &_tty);
}


nl()
{
/*
	VERSION 5.0
	_tty.c_iflag |= ICRNL;
	_tty.c_oflag |= ONLCR;
*/
	_tty.sg_flags |= CRMOD;
	NONL = TRUE;
	ioctl(_tty_ch, TIOCSETP, &_tty);
}


nonl()
{
/*
	VERSION 5.0
	_tty.c_iflag &= ~ICRNL;
	_tty.c_oflag &= ~ONLCR;
*/
	_tty.sg_flags &= ~CRMOD;
	NONL = FALSE;
	ioctl(_tty_ch, TIOCSETP, &_tty);
}

savetty()
{
	ioctl(_tty_ch, TIOCGETP, &_tty);
	_res_flg = _tty;
}

resetty()
{
	_tty = _res_flg;
	ioctl(_tty_ch, TIOCSETP, &_tty);
}
