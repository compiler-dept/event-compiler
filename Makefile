BIN=evc

CFLAGS=-O0 -g -Wall -std=gnu11 -Ilibcollect `llvm-config --cflags`
CXXFLAGS=-O0 -g -Wall -Ilibcollect `llvm-config --cxxflags`
LDFLAGS=-Llibcollect `llvm-config --ldflags`
LDLIBS=-lcollect `llvm-config --libs` -lcurses -lpthread -ldl
YACC=lemon/lemon
LEX=flex

SOURCES=src/evc.c src/compiler.c src/lexer.l src/parser.y src/ast.c src/scope.c src/validator.c src/codegen.c src/operators.c src/astdump.c
COBJECTS=$(patsubst %.c, %.o, $(SOURCES))
LOBJECTS=$(patsubst %.l, %.o, $(COBJECTS))
OBJECTS=$(patsubst %.y, %.o, $(LOBJECTS))

.PHONY: all clean lemon libcollect getexternals test valgrind docs docs\:preview docs\:deploy style

all: $(BIN)

$(BIN): $(OBJECTS) src/lexer.c libcollect
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS) $(LDLIBS)

src/evc.o: src/evc.c src/lexer.c

src/lexer.c: src/lexer.l src/parser.c
	$(LEX) --header-file=src/lexer.h -o $@ $<

src/parser.c: src/parser.y lemon
	$(YACC) $<

TEST_DEPS=$(filter-out src/%.l, $(filter-out src/%.y, $(filter-out src/evc.c src/codegen.c, $(SOURCES)))) src/lexer.c src/parser.c
TEST_SOURCES=$(wildcard tests/*.c)
TEST_CFLAGS=-g -Wall -std=gnu99 -Ilibcollect $(patsubst %, -Wno-%, $(DISABLED_WARNINGS))
TEST_LDFLAGS=-Llibcollect
TEST_LDLIBS=-lcollect -lm
tests/testsuite: $(TEST_SOURCES) $(TEST_DEPS) libcollect
	tests/generate.py tests
	$(CC) -Isrc -L. $(TEST_CFLAGS) -Wno-unused-function -o $@ $(TEST_SOURCES) $(TEST_DEPS) $(TEST_LDFLAGS) $(TEST_LDLIBS)

test: tests/testsuite
	tests/testsuite

valgrind: tests/testsuite
	valgrind --leak-check=full --error-exitcode=1 --suppressions=tests/valgrind.supp tests/testsuite

docs:
	mkdocs build --clean

docs\:preview: docs
	mkdocs serve

docs\:deploy: docs
	mkdocs gh-deploy

clean:
	rm -f $(BIN) $(OBJECTS) src/lexer.c src/lexer.h src/parser.c src/parser.h src/parser.out
	rm -rf tests/testsuite tests/testsuite.dSYM tests/.clarcache tests/clar.suite
	rm -rf doxygen/html
	rm -rf site

libcollect:
	@- make -C libcollect

lemon:
	@- make -C lemon

getexternals:
	git submodule init
	git submodule update

style:
	astyle -A3s4SpHk3jn "src/*.c" "src/*.h"
