# Makefile for rogue
# %W% (Berkeley) %G%
#
# Super-Rogue
# Copyright (C) 1984 Robert D. Kindelberger
# All rights reserved.
#
# Based on "Rogue: Exploring the Dungeons of Doom"
# Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
# All rights reserved.
#
# See the file LICENSE.TXT for full copyright and licensing information.

DISTNAME=srogue9.0-1
PROGRAM=srogue

HDRS= bob.h cx.h ncx.h rdk.h rogue.h 
OBJS= vers.o armor.o chase.o command.o daemon.o daemons.o disply.o encumb.o \
      fight.o global.o init.o io.o list.o main.o misc.o monsters.o move.o \
      new_leve.o options.o pack.o passages.o potions.o pstats.o rings.o rip.o \
      rooms.o save.o scrolls.o state.o sticks.o things.o trader.o weapons.o \
      wizard.o xcrypt.o
CFILES= vers.c armor.c chase.c command.c daemon.c daemons.c disply.c encumb.c \
      fight.c global.c init.c io.c list.c main.c misc.c monsters.c move.c \
      new_leve.c options.c pack.c passages.c potions.c pstats.c rings.c rip.c \
      rooms.c save.c scrolls.c state.c sticks.c things.c trader.c weapons.c \
      wizard.c xcrypt.c

MISC=	Makefile LICENSE.TXT rogue.nr

CC    = gcc
CFLAGS= -g
CRLIB = -lcurses
RM    = rm -f
TAR   = tar

$(PROGRAM): $(HDRS) $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(CRLIB) -o $@

tags: $(HDRS) $(CFILES)
	ctags -u $?
	ed - tags < :ctfix
	sort tags -o tags

lint:
	lint -hxbc $(CFILES) $(CRLIB) > linterrs

clean:
	rm -f $(OBJS) core 
	rm -f $(PROGRAM) $(PROGRAM).exe $(PROGRAM) $(PROGRAM).exe $(PROGRAM).tar $(PROGRAM).tar.gz $(PROGRAM).doc

count:
	wc -l $(HDRS) $(CFILES)

realcount:
	cc -E $(CFILES) | ssp - | wc -l

update:
	ar uv .SAVE $(CFILES) $(HDRS) $(MISC)

dist:
	@mkdir dist
	cp $(CFILES) $(HDRS) $(MISC) dist

dist.src:
	make clean
	tar cf $(DISTNAME)-src.tar $(CFILES) $(HDRS) $(MISC)
	gzip -f $(DISTNAME)-src.tar

dist.irix:
	make clean
	make CC=cc CFLAGS="-woff 1116 -O3" $(PROGRAM)
	tbl rogue.nr | nroff -mm | colcrt - > $(PROGRAM).doc
	tar cf $(DISTNAME)-irix.tar $(PROGRAM) LICENSE.TXT $(PROGRAM).doc
	gzip -f $(DISTNAME)-irix.tar

debug.aix:
	make clean
	make CC=xlc CFLAGS="-qmaxmem=16768 -g -DWIZARD  -qstrict" $(PROGRAM)

dist.aix:
	make clean
	make CC=xlc CFLAGS="-qmaxmem=16768 -O3 -qstrict" $(PROGRAM)
	tbl rogue.nr | nroff -mm | colcrt - > $(PROGRAM).doc
	tar cf $(DISTNAME)-aix.tar $(PROGRAM) LICENSE.TXT $(PROGRAM).doc
	gzip -f $(DISTNAME)-aix.tar

debug.linux:
	make clean
	make CFLAGS="-g -DWIZARD" $(PROGRAM)

dist.linux:
	make clean
	make $(PROGRAM)
	groff -P-c -t -mm -Tascii rogue.nr | sed -e 's/.\x08//g' >$(PROGRAM).doc
	tar cf $(DISTNAME)-linux.tar $(PROGRAM) LICENSE.TXT $(PROGRAM).doc
	gzip -f $(DISTNAME)-linux.tar
	
debug.interix: 
	make clean
	make CFLAGS="-g3 -DWIZARD" $(PROGRAM)
	
dist.interix: 
	make clean
	make $(PROGRAM)
	groff -P-b -P-u -t -mm -Tascii rogue.nr > $(PROGRAM).doc
	tar cf $(DISTNAME)-interix.tar $(PROGRAM) LICENSE.TXT $(PROGRAM).doc
	gzip -f $(DISTNAME)-interix.tar
	
debug.cygwin:
	make clean
	make CFLAGS="-g3 -DWIZARD" $(PROGRAM)

dist.cygwin:
	make clean
	make $(PROGRAM)
	groff -P-c -t -mm -Tascii rogue.nr | sed -e 's/.\x08//g' >$(PROGRAM).doc
	tar cf $(DISTNAME)-cygwin.tar $(PROGRAM).exe LICENSE.TXT $(PROGRAM).doc
	gzip -f $(DISTNAME)-cygwin.tar
	
debug.djgpp: 
	make clean
	make CFGLAGS="-g3 -DWIZARD" LDFLAGS="-L$(DJDIR)/LIB" CRLIB="-lpdcurses" $(PROGRAM)

dist.djgpp: 
	make clean
	make LDFLAGS="-L$(DJDIR)/LIB" CRLIB="-lpdcurses" $(PROGRAM)
	groff -t -mm -Tascii rogue.nr | sed -e 's/.\x08//g' > $(PROGRAM).doc
	rm -f $(DISTNAME)-djgpp.zip
	zip $(DISTNAME)-djgpp.zip $(PROGRAM).exe LICENSE.TXT $(PROGRAM).doc
