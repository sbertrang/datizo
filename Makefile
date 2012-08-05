
SUBDIR +=	lib
SUBDIR +=	bin

run debug rebug:
	cd bin && ${MAKE} ${.TARGET}

.include <bsd.subdir.mk>
