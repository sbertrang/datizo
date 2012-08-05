
PROG =		datizo

SRCS +=		main.c

MAN =


run: ${PROG}
	LD_LIBRARY_PATH=../lib ./${PROG}

debug: ${PROG} ${PROG}.core
	LD_LIBRARY_PATH=../lib gdb ./${PROG} ./${PROG}.core

rebug: ${PROG}
	LD_LIBRARY_PATH=../lib gdb ./${PROG}

.include <bsd.prog.mk>
