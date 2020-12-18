CC = gcc
CLFAGS = -Wall -Wextra -Wno-implicit-fallthrough -ggdb
LDLIBS = -lpthread

FILES = main.c common.h common.c mole_index.h mole_index.c indexer.h indexer.c cli.h cli.c
SOURCES = $(filter %.c,${FILES})

all: mole

mole: ${FILES}
	${CC} ${SOURCES} -o $@ ${CLFAGS} ${LDLIBS}

pack: ${FILES} Makefile
	tar -cjf brzozkak.etap$(ETAP).tar.bz2 $^

clean:
	rm mole

.PHONY: clean pack
