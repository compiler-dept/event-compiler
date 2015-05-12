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
    printf("Vector 1 [%d]:\n", ev->pos_len);
    for (int i = 0; i < ev->pos_len; i++){
        printf("Value %f\n", (ev->pos)[i]);
    }
    return EXIT_SUCCESS;
}
