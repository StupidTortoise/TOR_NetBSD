#	@(#)Makefile	8.2 (Berkeley) 1/2/94

CFLAGS+=#-DTFILE=\"/dev/ttyp0\"
CFLAGS+=-D_CURSES_PRIVATE -I${.CURDIR}
LIB=	curses
SRCS=	addbytes.c addch.c addnstr.c box.c clear.c clrtobot.c clrtoeol.c \
	cr_put.c ctrace.c cur_hash.c curses.c delch.c deleteln.c delwin.c \
	erase.c fullname.c getch.c getstr.c id_subwins.c idlok.c initscr.c \
	insch.c insertln.c longname.c move.c mvwin.c newwin.c overlay.c \
	overwrite.c printw.c putchar.c refresh.c scanw.c scroll.c setterm.c \
	standout.c toucholap.c touchwin.c tscroll.c tstp.c tty.c unctrl.c
MAN3=	curses.0

beforeinstall:
	-cd ${.CURDIR}; cmp -s curses.h ${DESTDIR}/usr/include/curses.h > \
	    /dev/null 2>&1 || \
	    install -c -o ${BINOWN} -g ${BINGRP} -m 444 curses.h \
	    ${DESTDIR}/usr/include

.if make(install)
SUBDIR+= PSD.doc
.endif

.include <bsd.lib.mk>
