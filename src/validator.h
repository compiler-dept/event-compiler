#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <tree.h>

/**
 * \file validator.h
 */

/**
 * \brief Validate a given AST
 *
 * \param root pointer to root node of AST
 * \return 1 if validation successful, 0 otherwise
 */
int validate(struct node *root);

#endif
