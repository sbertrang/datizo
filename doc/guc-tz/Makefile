
.SUFFIXES: .gv .pdf

mupdf: guc-tz.pdf
	mupdf ${.ALLSRC}

.gv.pdf:
	dot -T${.TARGET:E} -o${.TARGET} ${.IMPSRC}

