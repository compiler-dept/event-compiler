#include <stdint.h>

struct __attribute__((__packed__)) PositionEvent {
    int16_t time_len;
    double *time;
    int16_t position_len;
    double *position;
};

struct __attribute__((__packed__)) VelocityEvent {
    int16_t velocity_len;
    double *velocity;
};

uint8_t VelocityRule_active(struct PositionEvent *posEv1, struct PositionEvent *posEv2);

struct VelocityEvent *VelocityRule_function(struct PositionEvent *posEv1, struct PositionEvent *posEv2);
