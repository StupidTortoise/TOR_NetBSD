#	from: @(#)Makefile	8.1 (Berkeley) 6/1/93
#	$Id: Makefile,v 1.1 1994/05/24 07:33:47 glass Exp $

CFLAGS=		-DLIBC_SCCS
OBJS=		crt0.o gcrt0.o
CLEANFILES+=	core a.out

all: ${OBJS}

crt0.o: crt0.s
	${CPP} -DCRT0 ${.ALLSRC} | ${AS} -o $@
	@${LD} -x -r ${.TARGET}
	@mv a.out ${.TARGET}

gcrt0.o: crt0.s
	${CPP} -DMCRT0 ${.ALLSRC} | ${AS} -o $@
	@${LD} -x -r ${.TARGET}
	@mv a.out ${.TARGET}

install:
	install ${COPY} -o ${BINOWN} -g ${BINGRP} -m 444 ${OBJS} \
		${DESTDIR}/usr/lib

depend lint tags:

.include <bsd.prog.mk>
