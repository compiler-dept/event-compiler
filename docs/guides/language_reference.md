# Language Reference

## Introduction

This document is the primary reference for the **Event** programming language. It
describes each language construct.

This document does not serve as an introduction to the language itself.

## Input format

**Event** is interpreted as a sequence of ASCII characters. Every **statement**
has to be closed with a semicolon (```;```).

**Statements** are marked with the
tag ```Stamement``` across this document.

The following keywords are forbidden to use as identifiers and types:

- event
- extends
- predicate

The top level input sequence are **statements** terminated with a semicolon
(```;```).

## Comments

Comments in **Event** follow the C89 style comments. Starting with ```/*``` and
ending with ```*/```.

    <code>
    /* commented out block /*
    <code>

## Whitespaces

The interpretation of **Event** allows to use the following list of whitespaces:

- space, ' '
- tab, '\t'
- LF, '\n'
- CR, '\r'

## Identifiers

Identifiers are defined as follows:

- The first character has to be a small cased letter between a and z (```[a-z]```)
- The remaining characters can be small and capital letters, numbers and
underscores (```[a-zA-Z0-9_]*```)

Example:

    anIdentifier_42

## Types

Types for **Event** are defined as follows:

- The first character has to be a capital letter between A and Z (```[A-Z]```)
- The remaining characters can be small and capital letters, numbers and
underscores (```[a-zA-Z0-9_]*```)

Example:

    AType_23

## Numbers

**Event** handles numbers of the type ```double``` as defined by the used operating
system. They are defined as follows:

- The first line of characters have to be digits between 0 and 9 (```[0-9]+```)
- The remaining characters are optional and have to start with a dot (```.```)
followed by at least one digit (```(\.[0-9]+)?```)

Example:

    42
    1.337

## Vectors

Vectors are defined as a list of numbers, separated by commas and can only
contain numbers as a result of a given expression. The list is encapsulated by
square brackets (```[]```).

Example:

    [42]
    [1, 2, 1 + 2]

It is not possible to stack vectors into other vectors.

## Comparisons

In **Event**, you can only compare vectors. The following comparisons are
possible:

- Equal (```v1 == v2```)
- Not Equal (```v1 != v2```)
- Less than, every member (```v1 < v2```)
- Greater that, every member (```v1 > v2```)

Example:

    event1.pos == event2.pos
    event1.time > event2.time

## Operations

Like comparisons, operation can also operate only on vectors. Following
operations are possible, based on vector math:

- Addition (```v1 + v2```)
- Difference (```v1 - v2```)
- Scalar multiplication (```s * v```)

Example:

    event.pos + [1, 2, 3]
    [1, 2] - [3, 4]
    6 * event.time

Inside a vector as a member more mathematical operations are possible. Every
member operation is an expression which has to result in number. The following
operations are possible:

- Addition (```s1 + s2```)
- Difference (```s1 - s2```)
- Multiplication (```s1 * s2```)
- Division (```s1 / s2```)
- Negation (```-s```)

Multiplication and division have precedence over addition and difference.
Parenthesis (```()```) are possible to change the precedence.

Example:

    1 + 1
    4 - 3
    2 * 7
    13 / 3
    -42
    (1 + 2) * 3

## Events

```Statement```

Events are containers for related information. One event can extend another one
by using the keyword ```extends```. The event declaration has the following
structure:

- An event has to start with the keyword ```event``` followed by a whitespaces
- The second part has to be a ```Type``` as referred above

The third part and the fourth part are optional for event inheritance.

- The keyword ```extends```
- ```Type``` of the extended event

The following parts are mandatory and define a comma separated list of event
members holding the actual information and are encapsulated by curly braces
(```{}```).

- Opening curly brace ```{```
- Comma separated list of identifiers as described above
- Closing curly brace ```}```

Example:

    event SampleEvent { position, time };
    event RangeEvent extends SampleEvent { range };

For inheritance, only new members are allowed to be added in the inherited
event. It is forbidden to declare the same member in both, the event and it's
inheritance.

To define an event with content, you enclose the information with curly braces.

- Opening curly brace (```{```)

The following is a comma separated list:

- **Identifier**
- Equal sign (```=```)
- Vector

- Closing curly brace (```}```)

Example:

    {
        position = event_a.position + event_b.position,
        time = event_a.time + [1.34]
    }

## Predicates

```Statement```

Predicates are operating on events passed to them. They return either
```False``` or ```True```. The return value is generated by the comparison
applied to the predicate. A predicate has to take at least on event as an
argument. The expression assigned to the predicate has to be a single comparison
as described above. Predicates are created as below:

- A predicate has to start with the keyword ```predicate``` followed by a
whitespace
- The second is an **identifier** to name the predicate
- Opening parenthesis to begin the list of arguments (```(```)

Arguments are a comma separated list of tuples made out of a **type** and an
**identifier**. Arguments are defined as follows:

- **Type** as described above
- **Identifier** as described above

Back to the argument list.

- Closing parenthesis to end the list of arguments (```)```)

To separate the predicate definition from it's comparison expression, **"define"**
(```:=```) is used.

- Define sign (```:=```)
- Comparison as explained above

Example:

    predicate a_predicate(SampleEvent e) := e.position == [1, 1, 1];
    predicate b_predicate(SampleEvent e, RangeEvent r) := e.position != r.position;

## Functions

```Statement```

Functions are also operating on events passed to them. A function must have a
return type. Functions are created as follows:

- A function starts with a **type** as described earlier
- The second is an **identifier** which names the function
- Opening parenthesis to begin the list of arguments (```(```)

Arguments are a comma separated list of tuples made out of a **type** and an
**identifier** and optional. Arguments are defined as follows:

- **Type** as described above
- **Identifier** as described above

Back to the argument list.

- Closing parenthesis to end the list of arguments (```)```)

To separate the function definition from it's expression block, **"define"**
(```:=```) is used.

- Define sign (```:=```)
- Expression as described below

An expression can be one of the following:

- Function call (```f(x)```)
- Event (```{ position = [1, 2, 3] }```)

Example:

    SampleEvent function_a(SampleEvent event_a) := {
        position = event_a.position + [1, 0, 0],
        time = 6 * event_a.time
    }

    SampleEvent function_b(SampleEvent e) := function_a(e);

## Rules

Rules take a list of event **types**, a list of predicates and a call function.

- A **type** as rule name
- Colon (```:```)
- Opening square bracket (```[```)

The following is optional:

- Comma separated list of event types (```EventA, EventB, ...```)

The following is optional:

- Colon (```:```)
- Comma separated list of predicate name **identifiers**

The following is mandatory:

- Closing square bracket (```]```)
- Right arrow (```->```)
- Function name identifier (```function_a```)

Example:

    RuleA: [SampleEvent : a_predicate] -> function_b;
