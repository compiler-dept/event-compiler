BIN=compiler

DISABLED_WARNINGS=switch
CFLAGS=-g -Wall -std=gnu99 -Ilibcollect $(patsubst %, -Wno-%, $(DISABLED_WARNINGS))
LDFLAGS=-Llibcollect
LDLIBS=-lcollect
YACC=lemon/lemon
LEX=flex

SOURCES=src/compiler.c src/lexer.l src/parser.y src/ast.c
COBJECTS=$(patsubst %.c, %.o, $(SOURCES))
LOBJECTS=$(patsubst %.l, %.o, $(COBJECTS))
OBJECTS=$(patsubst %.y, %.o, $(LOBJECTS))

.PHONY: all clean lemon libcollect getexternals

all: $(BIN)

$(BIN): $(OBJECTS) src/lexer.c libcollect
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS) $(LDLIBS)

src/compiler.o: src/compiler.c src/lexer.c

src/lexer.c: src/lexer.l src/parser.c
	$(LEX) --header-file=src/lexer.h -o $@ $<

src/parser.c: src/parser.y lemon
	$(YACC) $<

TEST_DEPS=$(filter-out src/%.l, $(filter-out src/%.y, $(filter-out src/compiler.c, $(SOURCES)))) src/lexer.c src/parser.c
tests/testsuite: tests/testsuite.c $(TEST_DEPS) libcollect
	$(CC) -Isrc -L. $(CFLAGS) -o $@ $< $(TEST_DEPS) $(LDFLAGS) $(LDLIBS) -lcunit

test: tests/testsuite
	tests/testsuite
	valgrind --leak-check=full --error-exitcode=1 tests/testsuite

clean:
	rm -f $(BIN) $(OBJECTS) src/lexer.c src/lexer.h src/parser.c src/parser.h src/parser.out tests/testsuite
	rm -rf tests/testsuite.dSYM

libcollect:
	@- make -C libcollect

lemon:
	@- make -C lemon

getexternals:
	git submodule init
	git submodule update
