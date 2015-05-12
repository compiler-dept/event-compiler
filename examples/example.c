#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

struct __attribute__((__packed__)) SampleEvent {
    int16_t pos_len;
    double *pos;
    int16_t time_len;
    double *time;
};

struct SampleEvent *constfun(struct SampleEvent *p);

int main(){
    struct SampleEvent e;
    e.pos_len = 2;
    e.pos = malloc(2 * sizeof(double));
    e.pos[0] = 1;
    e.pos[1] = 1;

    struct SampleEvent *ev = constfun(&e);

    printf("Vector 1 [%d]:\n", ev->pos_len);
    for (int i = 0; i < ev->pos_len; i++){
        printf("Value %f\n", (ev->pos)[i]);
    }
    printf("Vector 2 [%d]:\n", ev->time_len);
    for (int i = 0; i < ev->time_len; i++){
        printf("Value %f\n", (ev->time)[i]);
    }
    return EXIT_SUCCESS;
}
