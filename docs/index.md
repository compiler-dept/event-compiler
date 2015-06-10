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

## Video Example

<iframe src="https://player.vimeo.com/video/130224735" width="500" height="296" frameborder="0" webkitallowfullscreen mozallowfullscreen allowfullscreen></iframe> <p><a href="https://vimeo.com/130224735">Building an Event-Processor with the Event-Language</a> from <a href="https://vimeo.com/apfohl">apfohl</a> on <a href="https://vimeo.com">Vimeo</a>.</p>

<iframe src="https://player.vimeo.com/video/130224734" width="500" height="297" frameborder="0" webkitallowfullscreen mozallowfullscreen allowfullscreen></iframe> <p><a href="https://vimeo.com/130224734">Using an Event-Processor with the Event-Language</a> from <a href="https://vimeo.com/apfohl">apfohl</a> on <a href="https://vimeo.com">Vimeo</a>.</p>

## Presentation Slides

<script async class="speakerdeck-embed" data-id="bdb12de10bb944c1901187b466d11239" data-ratio="1.33333333333333" src="//speakerdeck.com/assets/embed.js"></script>
