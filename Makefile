CFLAGS=-g -Wall $(patsubst %, -Wno-%, $(DISABLED_WARNINGS)) -std=gnu11 -Ilibcollect
LDFLAGS=-Llibcollect -lcollect
YACC=lemon/lemon
LEX=flex


.PHONY: all getexternals

getexternals:
	git submodule init
	git submodule update
