# Makefile
# 2016-12-19  Markku-Juhani O. Saarinen <mjos@iki.fi>

BIN	= dekryptize
OBJS	= dekryptize.o
DIST	= dekryptize

CC	= gcc
CFLAGS	= -Wall -Ofast
LIBS	= -lncurses
LDFLAGS	=
INCS	=

$(BIN):      $(OBJS)
	$(CC) $(LDFLAGS) -o $(BIN) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

clean:
	rm -rf $(DIST)-*.txz $(OBJS) $(BIN) *~

dist:	clean
	cd ..; \
	tar cfvJ $(DIST)/$(DIST)-`date -u "+%Y%m%d%H%M"`.txz $(DIST)/*

