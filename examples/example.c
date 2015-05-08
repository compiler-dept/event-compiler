#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

struct __attribute__((__packed__)) InheritanceEvent {
    int16_t pos_len;
    double *pos;
    int16_t time_len;
    double *time;
};

struct InheritanceEvent *constfun(void);

int main(){
    struct InheritanceEvent *ev = constfun();
    puts("Vector 1:");
    for (int i = 0; i < ev->pos_len; i++){
        printf("Value %f\n", (ev->pos)[i]);
    }
    puts("Vector 2:");
    for (int i = 0; i < ev->time_len; i++){
        printf("Value %f\n", (ev->time)[i]);
    }
    return EXIT_SUCCESS;
}
