BIN=event-compiler

DISABLED_WARNINGS=switch
CFLAGS=-g -Wall -std=gnu99 -Ilibcollect $(patsubst %, -Wno-%, $(DISABLED_WARNINGS))
LDFLAGS=-Llibcollect
LDLIBS=-lcollect
YACC=lemon/lemon
LEX=flex

SOURCES=src/event-compiler.c src/compiler.c src/lexer.l src/parser.y src/ast.c src/scope.c
COBJECTS=$(patsubst %.c, %.o, $(SOURCES))
LOBJECTS=$(patsubst %.l, %.o, $(COBJECTS))
OBJECTS=$(patsubst %.y, %.o, $(LOBJECTS))

.PHONY: all clean lemon libcollect getexternals

all: $(BIN)

$(BIN): $(OBJECTS) src/lexer.c libcollect
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS) $(LDLIBS)

src/event-compiler.o: src/event-compiler.c src/lexer.c

src/lexer.c: src/lexer.l src/parser.c
	$(LEX) --header-file=src/lexer.h -o $@ $<

src/parser.c: src/parser.y lemon
	$(YACC) $<

TEST_DEPS=$(filter-out src/%.l, $(filter-out src/%.y, $(filter-out src/event-compiler.c, $(SOURCES)))) src/lexer.c src/parser.c
TEST_SOURCES=$(wildcard tests/*.c)
tests/testsuite: $(TEST_SOURCES) $(TEST_DEPS) libcollect
	tests/generate.py tests
	$(CC) -Isrc -L. $(CFLAGS) -Wno-unused-function -o $@ $(TEST_SOURCES) $(TEST_DEPS) $(LDFLAGS) $(LDLIBS)

test: tests/testsuite
	tests/testsuite
	valgrind --leak-check=full --error-exitcode=1 --suppressions=tests/valgrind.supp tests/testsuite

clean:
	rm -f $(BIN) $(OBJECTS) src/lexer.c src/lexer.h src/parser.c src/parser.h src/parser.out
	rm -rf tests/testsuite tests/testsuite.dSYM tests/.clarcache tests/clar.suite

libcollect:
	@- make -C libcollect

lemon:
	@- make -C lemon

getexternals:
	git submodule init
	git submodule update
