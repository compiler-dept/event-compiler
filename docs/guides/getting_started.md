# Getting Started

## Prerequisites
The **Event Compiler** relies on a working installation of LLVM.  For Linux and OSX operating systems LLVM is easily installed using the package manager. For other
environments check [llvm.org](http://llvm.org/)

We currently use **version 3.6.1**

## Compiling the Compiler
Before we can get started using the **Event Compiler**, the source code needs to
be acquired and compiled. The simplest way of accomplishing this is to clone
directly from the [git repository](https://github.com/compiler-dept/event-compiler):

	git clone git@github.com:compiler-dept/event-compiler.git

Next, we navigate into the compiler's root directory. There, we fetch the external dependencies and start compilation by running the following commands:

```bash
cd event-compiler
make getexternals
make
```

It's as simple as that: Your compiler is up and running!


## Compiling Event Sources

Following the setup steps should have left you with the compiler binary **evc**.
This section explains how the compiler is used to compile **Event** sources to
platform specific assembler files.

An example of a basic **Event** source file is located in examples/simple.ev.
The **.ev** file extension is a convention, meaning you may use any other ending
of your choice. Compilation is a two-step process:

1. Generating llvm bytecode from the source file with **evc**
2. Compiling the llvm bytecode to a platform-specific assembler file with **llc**.

```bash
cd examples
./evc -i simple.ev -o simple.bc # writes bytecode to simple.bc
llc simple.bc # writes platform-specifc assembler to simple.s
```

### Commandline Parameters

- ``` -i <file> ``` Specify input file
- ``` -o <file>``` Specify output file; When missing dump llvm intermediate code to stdout
- ``` -a <file>``` Dump lisp-like representation of the ast to file
- ``` -V ``` Disable source validation; Use with caution: Compiling invalid code will probably lead to unexpected behavior
- ``` -C ``` Disable code generation
- ``` -h ``` Show help

### Cross Compiling

By default, **llc** generates assembler specific to your native machine. If you
wish to cross-compile for a different architecture, you may specify the target
using **llc**'s ```-mtriple``` commandline parameter. For further information
refer to the **llc** manpage.


## Calling the Native Code

The previous sections explained how to use the **Event Compiler** process *Event*
source files into platform-specific assembler files. This section explains how
to actually call the contained code from within a simple c application.


### Inferring Prototypes

As explained in [Concept](concept), the compiler generates native code containing
methods and struct definitions for each **rule** defined in the **Event** source code.

The following methods are created for each rule:

1. An activation function which returns an integer greater than zero if and only if predicates indicate the rule should be applied.
2. A processing function which uses the rule's parameters to generate a new event.

```C
int <<rule_name>>_active(struct MyEventType1 *event1, ..., struct MyEventTypeN *eventn)
struct MyReturnEvent *<<rule_name>>_function(struct MyEventType1 *event1, ..., struct MyEventTypeN *eventn)
```

For every event definition a struct is created. Since the members of our events are
vectors, each member is represented by an integer indicating its length and a pointer
to an accordingly sized array of double values.

```C
struct __attribute__((__packed__)) MyEventType1 {
    int16_t pos_len;
    double *pos;
    int16_t time_len;
    double *time;
};
```

Event inheritance is handled simply by prepending inherited fields to the struct.
Therefore:

```
event Parent { pos };
event Child extends Parent { time };
```

Results in:

```
struct __attribute__((__packed__)) Parent {
    int16_t pos_len;
    double *pos;
};

struct __attribute__((__packed__)) Child {
    int16_t pos_len;
    double *pos;
	int16_t time_len;
    double *time;
};
```

Both method prototypes and event struct definitions must be declared in the
calling program, in order to be linked against later.


### Compiling and Linking

Examples/simple.c contains a small program which uses the events and rules declared
in examples/simple.ev. The inferred prototypes are located in examples/simple.h.
In order to compile and link the program, ensure that you have followed the previous
steps to compile the **Event** source. Execute the following commands to compile
and link the example:

```bash
cd examples
gcc -o example example.c simple.s ../src/operators.c # clang works just as well
```

The additional source file **../src/operators.c** contains **Event**'s standard
library for vector arithmetics and is therefore required for the program to compile.

Run the example by executing:

```bash
./example
```

If everything worked out correctly, you should get the following output:

```
Vector 1 [3]:
Value 4.000000
Value 8.000000
Value 12.000000
Vector 2 [1]:
Value 0.493800
```
