# Using the Event Compiler

## Setup
Before we can get started using the **Event Compiler**, we need to collect and
compile its source code. The simplest way of accomplishing this is to clone
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
./evc -i examples/simple.ev -o simple.bc # writes bytecode to simple.bc
llc simple.bc # writes platform-specifc assembler to simple.s
```

### Commandline Parameters

- ``` -i ``` Specify input file
- ``` -o ``` Specify output file; When missing dump llvm intermediate code to stdout
- ``` -V ``` Disable source validation; Use with caution: Compiling invalid code will probably lead to unexpected behavior
- ``` -C ``` Disable code generation

### Cross Compiling

By default, **llc** generates assembler specific to your native machine. If you
wish to cross-compile for a different architecture, you may specify the target
using **llc**'s ```-mtriple``` commandline parameter. For further information
refer to the **llc** manpage.


## Calling the Native Code

The previous sections explained how to use the **Event Compiler** process *Event*
source files into platform-specific assembler files. This section explains how
to actually call the contained code from within a simple c application.

As explained in [Concept](concept), the compiler generates native code containing
two methods for each **rule** defined in the **Event** source code.

1. An activation function which returns an integer greater than zero if and only if predicates indicate rule should be applied.
2. A processing function which uses the rule's parameters to generate a new event.

```C
int <<rule_name>>_active(struct MyEventType1 *event1, ..., struct MyEventTypeN *eventn)
void <<rule_name>>_function(struct MyEventType1 *event1, ..., struct MyEventTypeN *eventn)
```
