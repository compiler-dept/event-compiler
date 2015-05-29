# Event

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
