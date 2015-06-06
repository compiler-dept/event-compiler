# Event

## Introduction
**Event** is a language for **rule based complex-event processing on distributed networks**.
It was designed to build a bridge between the C level interface
of event generating systems like sensors or actuators and the need for a
high level representation for event based systems. It provides the ability to
describe the processing of events on a higher level than fiddling with C or C++ code.

The Event Language is a compiled language. Therefore machine code is
generated during the process of compilation. This is achieved by using the LLVM
compiler collection, which provides us with a high level interface for
generating machine code for nearly all platforms supported by LLVM. LLVM is the
code generation backend for our event language front end.

## Example

From (**Event**):

    event SampleEvent { position, time };

    predicate p_position(SampleEvent a, SampleEvent b) := a.position > b.position;

    SampleEvent transform(SampleEvent a, SampleEvent b) :=
    {
        position = a.position + b.position,
        time = a.time + b.time
    };

    Transform: [SampleEvent, SampleEvent : p_position] -> transform;

To (**C**):

    int Transform_active(SampleEvent *, SampleEvent *);

    SampleEvent *Transform_function(SampleEvent *, SampleEvent *);
