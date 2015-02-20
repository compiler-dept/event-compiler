#ifndef PARSER_STATE_H
#define PARSER_STATE_H

#include <tree.h>

/**
 * \file parser_state.h
 */

/**
 * \brief Parser state
 *
 * These states are used to determine if a parse run was successful or not.
 */
enum states {
    ERROR = 1, /**< parsing failed */
    OK         /**< parsing ok */
};

/**
 * \brief Parser state structure
 *
 * This structure defines the state infomation for the lemon parser.
 */
struct parser_state {
    enum states state; /**< actual state of parser */
    struct node *root; /**< AST tree root node */
};

#endif
