#ifndef PARSER_STATE_H
#define PARSER_STATE_H

enum states {
    ERROR = 1,
    OK
};

struct parser_state {
    enum states state;
    struct node *root;
};

#endif
