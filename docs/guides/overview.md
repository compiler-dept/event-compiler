# Overview

## Motivation
![Missing Overview Image](../img/missing.png)

## Core Concepts
The **Event** language is comprised of four toplevel constructs. Together these
form an intuitive way of describing how events should be processed:

1. **Events** ([Syntax](language_reference/#events)) are the currency of data.
Processing and generating events, is the main task of this language. Events are
defined as composite types featuring an arbitrary number of vector fields.
Events can inherit other events, meaning that the parent-event's fields are
prepended to the child-event's fields. An event of a child type may be used
anywhere where an event of its parent type is valid.
2. **Functions** ([Syntax](language_reference/#functions)) receive a set of events
as input and use them to construct a new event. The input events remain unmodified.
3. **Predicates** ([Syntax](language_reference/#predicates)) like functions receive
a set of events as input and use them to generate a boolean decision based on
the values of the input events' fields.
4. **Rules** ([Syntax](language_reference/#rules)) combine the previous three
elements to define in which way events are processed. They consist of:

    - **Event types** defining **what** may be processed
    - **Predicates** defining **if** it is to be processed
    - A **Function** defining **how** it is to be processed.
