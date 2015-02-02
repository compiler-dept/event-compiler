#include <stdlib.h>
#include "ast.h"

void payload_free(struct payload *payload)
{
    if (payload) {
        // TODO

        free((struct payload *)payload);
    }
}
