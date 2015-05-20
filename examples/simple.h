#include <stdint.h>

struct __attribute__((__packed__)) SampleEvent {
    int16_t pos_len;
    double *pos;
    int16_t time_len;
    double *time;
};

uint8_t Rule_active(struct SampleEvent *e);

struct SampleEvent *Rule_function(struct SampleEvent *e);
