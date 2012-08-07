
UNAME !=	uname -s

.if ${UNAME} == "Darwin"
CWARNFLAGS +=	-Wall
DEBUG_FLAGS +=	-g
#WARNS =		1
#CFLAGS +=	-D_POSIX_SOURCE
.else
CDIAGFLAGS +=	-Wall
DEBUG +=	-g
WARNINGS =	Yes
.endif

