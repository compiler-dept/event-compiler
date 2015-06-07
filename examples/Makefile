CFLAGS = `llvm-config --cflags` -std=c11 -g
CXXFLAGS = `llvm-config --cxxflags`
LDFLAGS = `llvm-config --ldflags`
LDLIBS = `llvm-config --libs --system-libs`

all: example
	./example

example: gen.s example.c ../src/operators.c

gen.s: gen.bc
	llc gen.bc

gen.bc:
	@- make -C ..
	../evc -i velocity.ev -o gen.bc

clean:
	rm -f *.o *.ll *.s *.bc example
