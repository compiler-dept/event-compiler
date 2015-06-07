#include <stdio.h>
#include <stdlib.h>
#include "velocity.h"

int main(void) {
    struct PositionEvent posEv1;
    posEv1.time_len = 1;
    posEv1.time = malloc(posEv1.time_len * sizeof(double));
    posEv1.time[0] = 0;
    posEv1.position_len = 1;
    posEv1.position = malloc(posEv1.position_len * sizeof(double));
    posEv1.position[0] = 1;

    struct PositionEvent posEv2;
    posEv2.time_len = 1;
    posEv2.time = malloc(posEv2.time_len * sizeof(double));
    posEv2.time[0] = 2;
    posEv2.position_len = 1;
    posEv2.position = malloc(posEv2.position_len * sizeof(double));
    posEv2.position[0] = 6;

    if (VelocityRule_active(&posEv1, &posEv2)) {
        struct VelocityEvent *veloEv = VelocityRule_function(&posEv1, &posEv2);

        printf("Velocity: %f\n", veloEv->velocity[0]);

        free(veloEv->velocity);
        free(veloEv);
    }

    free(posEv1.time);
    free(posEv1.position);
    free(posEv2.time);
    free(posEv2.position);

    return EXIT_SUCCESS;
}
