#	$Id: Makefile,v 1.6 1993/10/05 21:39:25 pk Exp $

LIB=		groff
SRCS=		assert.cc change_lf.cc cmap.cc cset.cc device.cc errarg.cc\
		error.cc fatal.cc filename.cc font.cc fontfile.cc lf.cc\
		lineno.cc macropath.cc nametoindex.cc new.cc prime.cc\
	 	progname.cc ptable.cc searchpath.cc string.cc strsave.cc\
		tmpfile.cc illegal.cc version.cc
SRCS+=		iftoa.c itoa.c matherr.c
CFLAGS+=	-I$(.CURDIR)/../include

NOMAN=		noman
NOPROFILE=	noprofile	
NOPIC=		nopic

CLEANFILES+=	version.cc
version.cc: $(.CURDIR)/../VERSION
	@echo Making version.cc
	@echo const char \*version_string = \"`cat $(.CURDIR)/../VERSION`\"\; >$@

install:

.include "../Makefile.cfg"
.include <bsd.lib.mk>
