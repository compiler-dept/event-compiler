#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

struct __attribute__((__packed__)) InheritanceEvent {
    int16_t pos_len;
    double *pos;
    int16_t time_len;
    double *time;
    int16_t angle_len;
    double *angle;
};

struct InheritanceEvent *constfun(void);

int main(){
    struct InheritanceEvent *ev = constfun();
    printf("Vector 1 [%d]:\n", ev->pos_len);
    for (int i = 0; i < ev->pos_len; i++){
        printf("Value %f\n", (ev->pos)[i]);
    }
    printf("Vector 2 [%d]:\n", ev->time_len);
    for (int i = 0; i < ev->time_len; i++){
        printf("Value %f\n", (ev->time)[i]);
    }
    printf("Vector 3 [%d]:\n", ev->angle_len);
    for (int i = 0; i < ev->angle_len; i++){
        printf("Value %f\n", (ev->angle)[i]);
    }
    return EXIT_SUCCESS;
}
