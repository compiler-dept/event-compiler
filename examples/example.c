#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "simple.h"

int main(void) {
    struct SampleEvent e;
    e.pos_len = 3;
    e.pos = malloc(3 * sizeof(double));
    e.pos[0] = 1;
    e.pos[1] = 2;
    e.pos[2] = 3;
    e.time_len = 1;
    e.time = malloc(sizeof(double));
    e.time[0] = 0.12345;

    if (Rule_active(&e)) {
        struct SampleEvent *ev = Rule_function(&e);

        printf("Vector 1 [%d]:\n", ev->pos_len);
        for (int i = 0; i < ev->pos_len; i++){
            printf("Value %f\n", (ev->pos)[i]);
        }

        printf("Vector 2 [%d]:\n", ev->time_len);
        for (int i = 0; i < ev->time_len; i++){
            printf("Value %f\n", (ev->time)[i]);
        }
    }

    return EXIT_SUCCESS;
}
