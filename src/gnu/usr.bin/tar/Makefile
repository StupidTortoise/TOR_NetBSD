#	$Id: Makefile,v 1.9.2.1 1994/08/08 06:22:32 mycroft Exp $

PROG=	tar
SRCS=	buffer.c create.c diffarch.c extract.c fnmatch.c getdate.y \
	getoldopt.c getopt.c getopt1.c gnu.c list.c mangle.c names.c port.c \
	regex.c rtapelib.c tar.c update.c version.c
CFLAGS+= -DRETSIGTYPE=void -DDIRENT=1 -DHAVE_SYS_MTIO_H=1 -DHAVE_UNISTD_H=1
CFLAGS+= -DHAVE_GETGRGID=1 -DHAVE_GETPWUID=1 -DHAVE_STRING_H=1
CFLAGS+= -DHAVE_LIMITS_H=1 -DHAVE_STRSTR=1 -DHAVE_VALLOC=1 -DHAVE_MKDIR=1
CFLAGS+= -DHAVE_MKNOD=1 -DHAVE_RENAME=1 -DHAVE_FTRUNCATE=1 -DHAVE_GETCWD=1
CFLAGS+= -DHAVE_VPRINTF=1 -DNEEDPAD -I${.CURDIR}
CFLAGS+= -DDEF_AR_FILE=\"/dev/rst0\" -DDEFBLOCKING=20

LDSTATIC= -static
LDADD+=	-lgnumalloc
DPADD+=	/usr/lib/libgnumalloc.a

MAN1=tar.0
CLEANFILES+=y.tab.h

.include <bsd.prog.mk>
