#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Target.h>
#include <hashmap.h>
#include "codegen.h"

struct stack *function_arguments_stack = NULL;

void generate_function_definition(LLVMModuleRef module, struct node *node);

void generate_predicate_definition(LLVMModuleRef module, struct node *node);

void generate_expression(LLVMModuleRef module, LLVMBuilderRef builder,
                         LLVMValueRef target_value, struct node *node);

void generate_additive_expression(LLVMModuleRef module, LLVMBuilderRef builder,
                                  LLVMValueRef target_value, struct node *node);

void generate_multiplicative_expression(LLVMModuleRef module, LLVMBuilderRef builder,
                                        LLVMValueRef target_value, struct node *node);

void debug(LLVMModuleRef module, LLVMBuilderRef builder, const char *str)
{
    LLVMValueRef puts_fun = LLVMGetNamedFunction(module, "puts");
    LLVMValueRef arg = LLVMBuildGlobalStringPtr(builder, str, "");
    LLVMBuildCall(builder, puts_fun, &arg, 1, "");
}

LLVMTypeRef VectorType()
{
    LLVMTypeRef element_types[2];
    element_types[0] = LLVMInt16Type();
    element_types[1] = LLVMPointerType(LLVMDoubleType(), 0);
    return LLVMStructType(element_types, 2, 1);
}


int generate_event_fields(struct stack **members, struct node *node)
{
    int member_count = node->childv[0]->childc;

    for (int i = 0; i < member_count; i++) {
        /* pointer to array of values */
        stack_push(members, LLVMPointerType(LLVMDoubleType(), 0));
        /* number of values */
        stack_push(members, LLVMInt16Type());
    }

    struct payload *payload = node->payload;
    if (payload->event_declaration.type[1]
            && payload->event_declaration.parent_ref) {
        member_count += generate_event_fields(members,
                                              payload->event_declaration.parent_ref);
    }

    return member_count;
}

LLVMTypeRef generate_event_declaration(LLVMModuleRef module, struct node *node)
{
    struct payload *payload = node->payload;
    LLVMTypeRef event_type = LLVMStructCreateNamed(LLVMGetModuleContext(module),
                             payload->event_declaration.type[0]);

    struct stack *members = NULL;
    int member_count = generate_event_fields(&members, node);

    LLVMTypeRef member_types[2 * member_count];
    for (int i = 0; i < 2 * member_count; i += 2) {
        /* number of values */
        member_types[i] = stack_pop(&members);
        /* pointer to array of values */
        member_types[i + 1] = stack_pop(&members);
    }

    LLVMStructSetBody(event_type, member_types, 2 * member_count, 1);
    return event_type;
}

LLVMTypeRef generate_event_type_if_necessary(LLVMModuleRef module, struct node *event_decl)
{
    struct payload *event_decl_payload = (struct payload *) event_decl->payload;
    char *event_name = event_decl_payload->event_declaration.type[0];
    LLVMTypeRef type = LLVMGetTypeByName(module, event_name);
    if (type) {
        return type;
    } else {
        return generate_event_declaration(module, event_decl);
    }
}

int generate_parameter_list(LLVMModuleRef module, struct node *node, LLVMTypeRef **parameters)
{
    int parameter_count = node->childc;

    *parameters = malloc(parameter_count * sizeof(LLVMTypeRef));
    for (int i = 0; i < parameter_count; i++) {
        struct node *parameter = node->childv[i];
        struct payload *parameter_payload = parameter->payload;
        struct node *event = parameter_payload->parameter.event_ref;
        LLVMTypeRef event_type = generate_event_type_if_necessary(module, event);
        LLVMTypeRef parameter_type = LLVMPointerType(event_type, 0);
        (*parameters)[i] = parameter_type;
    }

    return parameter_count;
}

void generate_initializer(LLVMModuleRef module, LLVMBuilderRef builder,
                          LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    /* get pointer to appropriate field in event */
    int index_in_struct = 2 * payload->initializer.ref_index;
    LLVMValueRef vector_in_struct = LLVMBuildStructGEP(builder, target_value, index_in_struct, "");
    /* evaluate initializer expression */
    generate_expression(module, builder, vector_in_struct, node->childv[0]);
}

void generate_initializer_sequence(LLVMModuleRef module, LLVMBuilderRef builder,
                                   LLVMValueRef target_value, struct node *node)
{
    for (int i = 0; i < node->childc; i++) {
        generate_initializer(module, builder, target_value, node->childv[i]);
    }
}

void generate_event_definition(LLVMModuleRef module, LLVMBuilderRef builder,
                               LLVMValueRef target_value, struct node *node)
{
    LLVMTypeRef event_type = LLVMGetElementType(LLVMGetElementType(LLVMTypeOf(target_value)));
    LLVMValueRef event_struct = LLVMBuildMalloc(builder, event_type, "");
    generate_initializer_sequence(module, builder, event_struct, node->childv[0]);
    LLVMBuildStore(builder, event_struct, target_value);
}

void generate_component_sequence(LLVMModuleRef module, LLVMBuilderRef builder,
                                 LLVMValueRef target_value, struct node *node)
{
    struct node *expression_sequence = node->childv[0];
    LLVMValueRef indices[2];
    indices[0] = LLVMConstInt(LLVMInt16Type(), 0, 0);
    for (int i = 0; i < expression_sequence->childc; i++) {
        indices[1] = LLVMConstInt(LLVMInt16Type(), i, 0);
        LLVMValueRef element_ptr = LLVMBuildGEP(builder, target_value, indices, 2, "");
        generate_expression(module, builder, element_ptr, expression_sequence->childv[i]);
    }
}

void generate_vector(LLVMModuleRef module, LLVMBuilderRef builder,
                     LLVMValueRef target_value, struct node *node)
{
    /* store vector length */
    struct node *expression_sequence = node->childv[0]->childv[0];
    int count = expression_sequence->childc;
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt16Type(), count, 0), target_value);

    /* get pointer to double * in vector */
    LLVMValueRef indices[2];
    indices[0] = LLVMConstInt(LLVMInt16Type(), 1, 0);
    LLVMValueRef _vector = LLVMBuildGEP(builder, target_value, indices, 1, "");
    LLVMTypeRef dest_type = LLVMPointerType(LLVMDoubleType(), 0);
    dest_type = LLVMPointerType(dest_type, 0);
    LLVMValueRef vector = LLVMBuildBitCast(builder, _vector, dest_type, "");

    /* allocate memory on heap for evaluated vector components */
    LLVMTypeRef array_type = LLVMArrayType(LLVMDoubleType(), count);
    LLVMValueRef array = LLVMBuildMalloc(builder, array_type, "");

    /* evaluate vector components */
    generate_component_sequence(module, builder, array, node->childv[0]);

    /* store address of array to double * in vector */
    indices[0] = LLVMConstInt(LLVMInt16Type(), 0, 0);
    indices[1] = LLVMConstInt(LLVMInt16Type(), 0, 0);
    LLVMValueRef arry_ptr = LLVMBuildGEP(builder, array, indices, 2, "");
    LLVMBuildStore(builder, arry_ptr, vector);
}

void generate_number(LLVMModuleRef module, LLVMBuilderRef builder,
                     LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    LLVMValueRef value = LLVMConstReal(LLVMDoubleType(), payload->atomic.number);
    LLVMBuildStore(builder, value, target_value);
}

void generate_identifier(LLVMModuleRef module, LLVMBuilderRef builder,
                         LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    struct hashmap *function_arguments = stack_peek(function_arguments_stack);
    if (payload->atomic.identifier[1] == NULL) {
        /* identifier is reference to event */
        if (payload->atomic.ref) {
            struct node *parameter_node = payload->atomic.ref;
            struct payload *parameter_payload = parameter_node->payload;
            char *key = parameter_payload->parameter.identifier;
            LLVMValueRef parameter = hashmap_get(function_arguments, key);
            if (parameter) {
                LLVMBuildStore(builder, parameter, target_value);
            }
        }
    } else {
        /* identifier is reference to field in event */
        if (payload->atomic.ref) {
            struct node *parameter_node = payload->atomic.ref;
            struct payload *parameter_payload = parameter_node->payload;
            char *key = parameter_payload->parameter.identifier;
            LLVMValueRef parameter = hashmap_get(function_arguments, key);

            int index_in_struct = 2 * payload->atomic.ref_index;
            LLVMValueRef vector_ptr = LLVMBuildStructGEP(builder, parameter, index_in_struct, "");
            LLVMValueRef value = LLVMBuildLoad(builder, vector_ptr, "");
            LLVMBuildStore(builder, value, target_value);

            LLVMValueRef index = LLVMConstInt(LLVMInt16Type(), 1, 0);
            vector_ptr = LLVMBuildGEP(builder, vector_ptr, &index, 1, "");
            target_value = LLVMBuildGEP(builder, target_value, &index, 1, "");
            LLVMTypeRef double_ptr_ptr = LLVMPointerType(LLVMPointerType(LLVMDoubleType(), 0), 0);
            vector_ptr = LLVMBuildBitCast(builder, vector_ptr, double_ptr_ptr, "");
            target_value = LLVMBuildBitCast(builder, target_value, double_ptr_ptr, "");
            value = LLVMBuildLoad(builder, vector_ptr, "");
            LLVMBuildStore(builder, value, target_value);
        }
    }
}

void generate_function_call(LLVMModuleRef module, LLVMBuilderRef builder,
                            LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    struct node *function_definition = payload->function_call.ref;
    struct payload *function_payload = function_definition->payload;
    struct node *expression_sequence = node->childv[0]->childv[0];

    /* ensure target function has been generated */
    generate_function_definition(module, function_definition);

    LLVMValueRef function = LLVMGetNamedFunction(module, payload->function_call.identifier);
    int parameter_count = 0;
    LLVMValueRef *args = NULL;

    if (function_payload->alternative == ALT_PARAMETER_LIST) {
        parameter_count = function_definition->childv[0]->childc;
        args = malloc(parameter_count * sizeof(LLVMValueRef));

        for (int i = 0; i < parameter_count; i++) {
            struct node *parameter = function_definition->childv[0]->childv[i];
            struct payload *parameter_payload = parameter->payload;
            struct node *event = parameter_payload->parameter.event_ref;
            LLVMTypeRef parameter_type = generate_event_type_if_necessary(module, event);
            parameter_type = LLVMPointerType(parameter_type, 0);

            /* allocate memory on stack for evaluated parameter */
            LLVMValueRef value = LLVMBuildAlloca(builder, parameter_type, "");

            /* evaluate parameter */
            generate_expression(module, builder, value, expression_sequence->childv[i]);

            args[i] = LLVMBuildLoad(builder, value, "");
        }
    }

    /* call function on parameters */
    LLVMValueRef result = LLVMBuildCall(builder, function, args, parameter_count, "");

    LLVMBuildStore(builder, result, target_value);

    free(args);
}

void generate_atomic(LLVMModuleRef module, LLVMBuilderRef builder,
                     LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    switch (payload->alternative) {
        case ALT_EVENT_DEFINITION:
            generate_event_definition(module, builder, target_value, node->childv[0]);
            break;
        case ALT_VECTOR:
            generate_vector(module, builder, target_value, node->childv[0]);
            break;
        case ALT_NUMBER:
            generate_number(module, builder, target_value, node);
            break;
        case ALT_IDENTIFIER:
            generate_identifier(module, builder, target_value, node);
            break;
        case ALT_FUNCTION_CALL:
            generate_function_call(module, builder, target_value, node->childv[0]);
            break;
        default:
            puts("ERROR NOT IMPLEMENTED YET");
    }
}

void generate_primary_expression(LLVMModuleRef module, LLVMBuilderRef builder,
                                 LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    switch (payload->alternative) {
        case ALT_EXPRESSION:
            generate_expression(module, builder, target_value, node->childv[0]);
            break;
        case ALT_ATOMIC:
            generate_atomic(module, builder, target_value, node->childv[0]);
            break;
        default:
            puts("ERROR NOT IMPLEMENTED YET");
    }
}

void generate_pow(LLVMModuleRef module, LLVMBuilderRef builder,
                  LLVMValueRef target_value, struct node *node)
{
    if (node->childc > 1) {
        LLVMValueRef base_ptr = LLVMBuildAlloca(builder, VectorType(), "");
        base_ptr = LLVMBuildStructGEP(builder, base_ptr, 0, "");
        LLVMValueRef exp_ptr = LLVMBuildAlloca(builder, LLVMDoubleType(), "");

        generate_primary_expression(module, builder, base_ptr, node->childv[0] );
        generate_primary_expression(module, builder, exp_ptr, node->childv[1] );

        LLVMValueRef base = LLVMBuildBitCast(builder, base_ptr, LLVMPointerType(LLVMInt8Type(), 0), "");
        LLVMValueRef exponent = LLVMBuildLoad(builder, exp_ptr, "");

        LLVMValueRef args[] = { base, exponent };
        LLVMValueRef function;

        function = LLVMGetNamedFunction(module, "op_v_pow_s");
        LLVMValueRef result = LLVMBuildCall(builder, function, args, 2, "");
        LLVMBuildStore(builder, result, target_value);

    } else {
        generate_primary_expression(module, builder, target_value, node->childv[0]);
    }
}

void generate_negation(LLVMModuleRef module, LLVMBuilderRef builder,
                       LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    if (payload->alternative  == ALT_POWER) {
        generate_pow(module, builder, target_value, node->childv[0]);
    } else if (payload->alternative  == ALT_NEGATION) {
        if (LLVMGetElementType(LLVMTypeOf(target_value)) == LLVMDoubleType()) {
            /* allocate memory on stack for evaluated operand */
            LLVMValueRef value_ptr = LLVMBuildAlloca(builder, LLVMDoubleType(), "");

            /* evaluate operand */
            generate_negation(module, builder, value_ptr, node->childv[0]);

            LLVMValueRef value = LLVMBuildLoad(builder, value_ptr, "");
            LLVMValueRef neg_one = LLVMConstReal(LLVMDoubleType(), -1);

            /* perform negation */
            LLVMValueRef result = LLVMBuildFMul(builder, neg_one, value, "");
            LLVMBuildStore(builder, result, target_value);
        }
    }
}

void handle_number_multiplication(LLVMBuilderRef builder,
                                  LLVMValueRef left_value_ptr, LLVMValueRef right_value_ptr,
                                  LLVMValueRef target_value, struct payload *payload)
{
    LLVMValueRef left_value = LLVMBuildLoad(builder, left_value_ptr, "");
    LLVMValueRef right_value = LLVMBuildLoad(builder, right_value_ptr, "");

    LLVMValueRef result;
    if (payload->alternative == ALT_MULT) {
        result = LLVMBuildFMul(builder, left_value, right_value, "");
    } else {
        result = LLVMBuildFDiv(builder, left_value, right_value, "");
    }
    LLVMBuildStore(builder, result, target_value);
}

void handle_vector_multiplication(LLVMModuleRef module, LLVMBuilderRef builder,
                                  LLVMValueRef left_value_ptr, LLVMValueRef right_value_ptr,
                                  LLVMValueRef target_value,
                                  struct payload *payload)
{

    LLVMValueRef left_value = LLVMBuildLoad(builder, left_value_ptr, "");
    LLVMValueRef right_value_ptr_casted = LLVMBuildBitCast(builder, right_value_ptr, LLVMPointerType(LLVMInt8Type(), 0), "");
    LLVMValueRef args[] = { left_value, right_value_ptr_casted };
    LLVMValueRef function;

    if (payload->alternative == ALT_MULT) {
        function = LLVMGetNamedFunction(module, "op_s_mult_v");
        LLVMValueRef result = LLVMBuildCall(builder, function, args, 2, "");

        result = LLVMBuildBitCast(builder, result, LLVMPointerType(LLVMInt16Type(), 0), "");
        LLVMValueRef count = LLVMBuildLoad(builder, result, "");
        LLVMBuildStore(builder, count, target_value);

        LLVMValueRef indices[] = { LLVMConstInt(LLVMInt16Type(), 1, 0) };
        result = LLVMBuildGEP(builder, result, indices, 1, "");
        LLVMTypeRef double_ptr_ptr = LLVMPointerType(LLVMPointerType(LLVMDoubleType(), 0), 0);
        result = LLVMBuildBitCast(builder, result, double_ptr_ptr, "");
        result = LLVMBuildLoad(builder, result, "");

        target_value = LLVMBuildGEP(builder, target_value, indices, 1, "");
        target_value = LLVMBuildBitCast(builder, target_value, double_ptr_ptr, "");
        LLVMBuildStore(builder, result, target_value);
    } else {
        debug(module, builder, "ERROR: Scalar / Vector and Vector / Scalar are undefined. Validation should have detected this.");
        debug(module, builder, "Will most likely segfault... now!");
    }
}


void generate_multiplication(LLVMModuleRef module, LLVMBuilderRef builder,
                             LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    LLVMValueRef left_value_ptr, right_value_ptr;

    int is_vector = LLVMGetElementType(LLVMTypeOf(target_value)) != LLVMDoubleType();

    /* allocate memory on stack for evaluated operands */
    if (is_vector) {
        left_value_ptr = LLVMBuildAlloca(builder, LLVMDoubleType(), "");
        right_value_ptr = LLVMBuildAlloca(builder, VectorType(), "");
        right_value_ptr = LLVMBuildStructGEP(builder, right_value_ptr, 0, "");
    } else {
        left_value_ptr = LLVMBuildAlloca(builder, LLVMDoubleType(), "");
        right_value_ptr = LLVMBuildAlloca(builder, LLVMDoubleType(), "");
    }

    /* evaluate operands */
    generate_multiplicative_expression(module, builder, left_value_ptr, node->childv[0] );
    generate_negation(module, builder, right_value_ptr, node->childv[1] );

    /* perform multiplication */
    if (!is_vector) {
        handle_number_multiplication(builder, left_value_ptr, right_value_ptr, target_value, payload);
    } else {
        handle_vector_multiplication(module, builder, left_value_ptr, right_value_ptr, target_value, payload);
    }
}

void generate_multiplicative_expression(LLVMModuleRef module, LLVMBuilderRef builder,
                                        LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    switch (payload->alternative) {
        case ALT_NEGATION:
            generate_negation(module, builder, target_value, node->childv[0]);
            break;
        case ALT_MULTIPLICATION:
            generate_multiplication(module, builder, target_value, node->childv[0]);
            break;
        default:
            puts("ERROR NOT IMPLEMENTED YET");
    }
}

void handle_number_addition(LLVMBuilderRef builder,
                            LLVMValueRef left_value_ptr, LLVMValueRef right_value_ptr,
                            LLVMValueRef target_value, struct payload *payload)
{
    LLVMValueRef left_value = LLVMBuildLoad(builder, left_value_ptr, "");
    LLVMValueRef right_value = LLVMBuildLoad(builder, right_value_ptr, "");

    LLVMValueRef result;
    if (payload->alternative == ALT_ADD) {
        result = LLVMBuildFAdd(builder, left_value, right_value, "");
    } else {
        result = LLVMBuildFSub(builder, left_value, right_value, "");
    }
    LLVMBuildStore(builder, result, target_value);
}

void handle_vector_addition(LLVMModuleRef module, LLVMBuilderRef builder,
                            LLVMValueRef left_value_ptr, LLVMValueRef right_value_ptr,
                            LLVMValueRef target_value,
                            struct payload *payload)
{

    LLVMValueRef left_value_ptr_casted = LLVMBuildBitCast(builder, left_value_ptr, LLVMPointerType(LLVMInt8Type(), 0), "");
    LLVMValueRef right_value_ptr_casted = LLVMBuildBitCast(builder, right_value_ptr, LLVMPointerType(LLVMInt8Type(), 0), "");
    LLVMValueRef args[] = {left_value_ptr_casted, right_value_ptr_casted};
    LLVMValueRef function;
    if (payload->alternative == ALT_ADD) {
        function = LLVMGetNamedFunction(module, "op_v_add_v");
    } else {
        function = LLVMGetNamedFunction(module, "op_v_sub_v");
    }

    LLVMValueRef result = LLVMBuildCall(builder, function, args, 2, "");
    result = LLVMBuildBitCast(builder, result, LLVMPointerType(LLVMInt16Type(), 0), "");
    LLVMValueRef count = LLVMBuildLoad(builder, result, "");
    LLVMBuildStore(builder, count, target_value);

    LLVMValueRef indices[] = { LLVMConstInt(LLVMInt16Type(), 1, 0) };
    result = LLVMBuildGEP(builder, result, indices, 1, "");
    LLVMTypeRef double_ptr_ptr = LLVMPointerType(LLVMPointerType(LLVMDoubleType(), 0), 0);
    result = LLVMBuildBitCast(builder, result, double_ptr_ptr, "");
    result = LLVMBuildLoad(builder, result, "");

    target_value = LLVMBuildGEP(builder, target_value, indices, 1, "");
    target_value = LLVMBuildBitCast(builder, target_value, double_ptr_ptr, "");
    LLVMBuildStore(builder, result, target_value);
}

void generate_addition(LLVMModuleRef module, LLVMBuilderRef builder,
                       LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    LLVMTypeRef expected_type;
    LLVMValueRef left_value_ptr, right_value_ptr;

    int is_vector = LLVMGetElementType(LLVMTypeOf(target_value)) != LLVMDoubleType();

    if (is_vector) {
        expected_type = VectorType();
    } else {
        expected_type = LLVMDoubleType();
    }

    /* allocate memory on stack for evaluated operands */
    left_value_ptr = LLVMBuildAlloca(builder, expected_type, "");
    right_value_ptr = LLVMBuildAlloca(builder, expected_type, "");

    if (is_vector) {
        left_value_ptr = LLVMBuildStructGEP(builder, left_value_ptr, 0, "");
        right_value_ptr = LLVMBuildStructGEP(builder, right_value_ptr, 0, "");
    }

    /* evaluate operands */
    generate_additive_expression(module, builder, left_value_ptr, node->childv[0] );
    generate_multiplicative_expression(module, builder, right_value_ptr, node->childv[1] );

    /* perform addition */
    if (!is_vector) {
        handle_number_addition(builder, left_value_ptr, right_value_ptr, target_value, payload);
    } else {
        handle_vector_addition(module, builder, left_value_ptr, right_value_ptr, target_value, payload);
    }
}

void generate_additive_expression(LLVMModuleRef module, LLVMBuilderRef builder,
                                  LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    switch (payload->alternative) {
        case ALT_MULTIPLICATIVE_EXPRESSION:
            generate_multiplicative_expression(module, builder, target_value, node->childv[0]);
            break;
        case ALT_ADDITION:
            generate_addition(module, builder, target_value, node->childv[0]);
            break;
        default:
            puts("ERROR NOT IMPLEMENTED YET");
    }
}

void generate_comparison_expression(LLVMModuleRef module, LLVMBuilderRef builder,
                                    LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    if (payload->alternative == ALT_ADDITIVE_EXPRESSION) {
        generate_additive_expression(module, builder, target_value, node->childv[0]);
    } else {
        LLVMTypeRef op_type = VectorType();

        /* allocate memory on stack for evaluated operands */
        LLVMValueRef left_op = LLVMBuildAlloca(builder, op_type, "");
        left_op = LLVMBuildStructGEP(builder, left_op, 0, "");
        LLVMValueRef right_op = LLVMBuildAlloca(builder, op_type, "");
        right_op = LLVMBuildStructGEP(builder, right_op, 0, "");

        /* evaluate operands */
        generate_additive_expression(module, builder, left_op, node->childv[0]);
        generate_additive_expression(module, builder, right_op, node->childv[1]);


        /* select appropriate comparison function */
        LLVMValueRef function = NULL;

        switch (payload->alternative) {
            case ALT_EQ:
                function = LLVMGetNamedFunction(module, "op_v_eq_v");
                break;
            case ALT_NEQ:
                function = LLVMGetNamedFunction(module, "op_v_neq_v");
                break;
            case ALT_GT:
                function = LLVMGetNamedFunction(module, "op_v_gt_v");
                break;
            case ALT_LT:
                function = LLVMGetNamedFunction(module, "op_v_lt_v");
                break;
            default:
                puts("ERROR NOT IMPLEMENTED YET");
                return;
        }

        /* call comparsion function */
        left_op = LLVMBuildBitCast(builder, left_op, LLVMPointerType(LLVMInt8Type(), 0), "");
        right_op = LLVMBuildBitCast(builder, right_op, LLVMPointerType(LLVMInt8Type(), 0), "");
        LLVMValueRef args[] = { left_op, right_op };
        LLVMValueRef result = LLVMBuildCall(builder, function, args, 2, "");

        LLVMBuildStore(builder, result, target_value);
    }
}

void generate_expression(LLVMModuleRef module, LLVMBuilderRef builder,
                         LLVMValueRef target_value, struct node *node)
{
    generate_comparison_expression(module, builder, target_value, node->childv[0]);
}

void generate_function_definition(LLVMModuleRef module, struct node *node)
{
    struct payload *payload = node->payload;

    if (!LLVMGetNamedFunction(module, payload->function_definition.identifier)) {
        /* function does not already exist */

        LLVMTypeRef return_event_type = generate_event_type_if_necessary(module,
                                        payload->function_definition.event_ref);
        LLVMTypeRef return_type = LLVMPointerType(return_event_type, 0);
        LLVMTypeRef function_type;
        int parameter_count = 0;

        if (payload->alternative == ALT_PARAMETER_LIST) {
            /* function has parameters */
            LLVMTypeRef *parameters = NULL;
            parameter_count = generate_parameter_list(module, node->childv[0], &parameters);
            function_type = LLVMFunctionType(return_type, parameters, parameter_count, 0);
            free(parameters);
        } else {
            /* function has no parameters */
            function_type = LLVMFunctionType(return_type, NULL, 0, 0);
        }

        LLVMValueRef function = LLVMAddFunction(module, payload->function_definition.identifier,
                                                function_type);

        LLVMValueRef arg_values[parameter_count];
        LLVMGetParams(function, arg_values);

        struct hashmap *function_arguments = NULL;

        for (int i = 0; i < parameter_count; i++) {
            struct payload *parameter_payload = node->childv[0]->childv[i]->payload;
            char *key = parameter_payload->parameter.identifier;
            hashmap_put(&function_arguments, key, arg_values[i]);
        }

        if (function_arguments) {
            stack_push(&function_arguments_stack, function_arguments);
        }

        LLVMBuilderRef builder = LLVMCreateBuilder();
        LLVMBasicBlockRef basic_block = LLVMAppendBasicBlock(function, "");
        LLVMPositionBuilderAtEnd(builder, basic_block);

        LLVMValueRef return_ptr = LLVMBuildAlloca(builder, return_type, "");

        if (payload->alternative == ALT_PARAMETER_LIST) {
            generate_expression(module, builder, return_ptr, node->childv[1]);
        } else {
            generate_expression(module, builder, return_ptr, node->childv[0]);
        }

        if (function_arguments) {
            hashmap_free(&function_arguments, NULL);
            stack_pop(&function_arguments_stack);
        }

        LLVMValueRef return_val = LLVMBuildLoad(builder, return_ptr, "");
        LLVMBuildRet(builder, return_val);
        LLVMDisposeBuilder(builder);
    }
}

void generate_predicate_definition(LLVMModuleRef module, struct node *node)
{
    struct payload *payload = node->payload;

    if (!LLVMGetNamedFunction(module, payload->predicate_definition.identifier)) {
        LLVMTypeRef return_type = LLVMInt8Type();
        LLVMTypeRef function_type;
        LLVMTypeRef *parameters = NULL;
        int parameter_count = generate_parameter_list(module, node->childv[0], &parameters);
        function_type = LLVMFunctionType(return_type, parameters, parameter_count, 0);
        free(parameters);

        LLVMValueRef function = LLVMAddFunction(module, payload->predicate_definition.identifier,
                                                function_type);

        LLVMValueRef arg_values[parameter_count];
        LLVMGetParams(function, arg_values);

        struct hashmap *function_arguments = NULL;

        for (int i = 0; i < parameter_count; i++) {
            struct payload *parameter_payload = node->childv[0]->childv[i]->payload;
            char *key = parameter_payload->parameter.identifier;
            hashmap_put(&function_arguments, key, arg_values[i]);
        }

        if (function_arguments) {
            stack_push(&function_arguments_stack, function_arguments);
        }

        LLVMBuilderRef builder = LLVMCreateBuilder();
        LLVMBasicBlockRef basic_block = LLVMAppendBasicBlock(function, "");
        LLVMPositionBuilderAtEnd(builder, basic_block);

        LLVMValueRef return_int = LLVMBuildAlloca(builder, return_type, "");

        generate_expression(module, builder, return_int, node->childv[1]);

        if (function_arguments) {
            hashmap_free(&function_arguments, NULL);
            stack_pop(&function_arguments_stack);
        }

        LLVMValueRef return_val = LLVMBuildLoad(builder, return_int, "");
        LLVMBuildRet(builder, return_val);
        LLVMDisposeBuilder(builder);
    }
}

/*
* Create the function for a rule. This function takes a sequence of events as inputs
* and returns an event.
*/
void generate_rule_function(LLVMModuleRef module, struct node *node, int parameter_count, LLVMTypeRef *parameters)
{
    struct payload *payload = node->payload;

    char function_name[1 + 9 + strlen(payload->rule_declaration.name)];
    sprintf(function_name, "%s_function", payload->rule_declaration.name);

    /* get return event type for rule function */
    struct payload *func_payload = ((struct payload *)payload->rule_declaration.ref->payload);
    struct node *event = func_payload->function_definition.event_ref;

    /* create return type for rule function and create it if necessary */
    LLVMTypeRef return_type = LLVMPointerType(generate_event_type_if_necessary(module, event), 0);

    /* create header for rule function */
    LLVMTypeRef function_type = LLVMFunctionType(return_type, parameters, parameter_count, 0);
    LLVMValueRef rule_function = LLVMAddFunction(module, function_name, function_type);

    /* get arguments for rule function */
    LLVMValueRef rule_arg_values[parameter_count];
    LLVMGetParams(rule_function, rule_arg_values);

    /* rule body builder */
    LLVMBuilderRef rule_builder = LLVMCreateBuilder();
    LLVMBasicBlockRef rule_basic_block = LLVMAppendBasicBlock(rule_function, "");
    LLVMPositionBuilderAtEnd(rule_builder, rule_basic_block);

    /* create assigned function if necessary */
    generate_function_definition(module, payload->rule_declaration.ref);
    /* get pinter to rule function */
    LLVMValueRef called_function = LLVMGetNamedFunction(module, payload->rule_declaration.identifier);

    /* call rule function */
    LLVMValueRef rule_result = LLVMBuildCall(rule_builder, called_function, rule_arg_values, parameter_count, "");

    /* build return value */
    LLVMBuildRet(rule_builder, rule_result);

    LLVMDisposeBuilder(rule_builder);
}

/*
* Create the active function for a rule. This function takes a sequence of events
* as input and returns 0, if one or more of the rule's predicates is false.
*/
void generate_rule_active_function(LLVMModuleRef module, struct node *node, int parameter_count, LLVMTypeRef *parameters)
{
    struct payload *payload = node->payload;

    /* create name for active function */
    char active_name[1 + 7 + strlen(payload->rule_declaration.name)];
    sprintf(active_name, "%s_active", payload->rule_declaration.name);

    /* create header for active function */
    LLVMTypeRef function_type = LLVMFunctionType(LLVMInt8Type(), parameters, parameter_count, 0);
    LLVMValueRef active_function = LLVMAddFunction(module, active_name, function_type);

    /* get arguments for active function */
    LLVMValueRef active_arg_values[parameter_count];
    LLVMGetParams(active_function, active_arg_values);

    /* active body builder */
    LLVMBuilderRef active_builder = LLVMCreateBuilder();
    LLVMBasicBlockRef active_basic_block = LLVMAppendBasicBlock(active_function, "");
    LLVMPositionBuilderAtEnd(active_builder, active_basic_block);

    LLVMValueRef active_result;

    if (((struct payload *)node->childv[0]->payload)->alternative == ALT_PREDICATE_SEQUENCE) {
        /* predicate list is present */

        struct payload *predicate_payload = NULL;

        LLVMValueRef count = LLVMConstInt(LLVMInt8Type(), node->childv[0]->childv[1]->childc, 0);
        LLVMValueRef sum = LLVMConstInt(LLVMInt8Type(), 0, 0);

        for (int i = 0; i < node->childv[0]->childv[1]->childc; i++) {
            predicate_payload = node->childv[0]->childv[1]->childv[i]->payload;

            /* generate predicate if necassery */
            generate_predicate_definition(module, predicate_payload->predicate.ref);

            /* get predicate function */
            LLVMValueRef predicate_function = LLVMGetNamedFunction(module, predicate_payload->predicate.identifier);

            /* call predicate function */
            LLVMValueRef predicate_result = LLVMBuildCall(active_builder, predicate_function, active_arg_values, parameter_count, "");

            sum = LLVMBuildAdd(active_builder, sum, predicate_result, "");
        }

        LLVMValueRef eq = LLVMGetNamedFunction(module, "op_i_eq_i");

        LLVMValueRef eq_args[] = { count, sum };

        active_result = LLVMBuildCall(active_builder, eq, eq_args, 2, "");
    } else {
        /* no predicate list ist present */
        active_result = LLVMConstInt(LLVMInt8Type(), 1, 0);
    }

    LLVMBuildRet(active_builder, active_result);
    LLVMDisposeBuilder(active_builder);
}

void generate_rule_declaration(LLVMModuleRef module, struct node *node)
{
    /* generate parameter list and create necessary event types */
    struct node *event_sequence = node->childv[0]->childv[0];
    LLVMTypeRef parameters[event_sequence->childc];
    for (int i = 0; i < event_sequence->childc; i++) {
        struct node *event = event_sequence->childv[i];
        struct payload *event_payload = event->payload;
        parameters[i] = LLVMPointerType(generate_event_type_if_necessary(module, event_payload->event.ref), 0);
    }

    generate_rule_active_function(module, node, event_sequence->childc, parameters);
    generate_rule_function(module, node, event_sequence->childc, parameters);
}


/*
* Declares LLVM signatures of methods defined in c-code.
* See operators.(c|h)
*/
void declare_external_methods(LLVMModuleRef module)
{
    /* Puts for debugging */
    LLVMTypeRef args[] = {LLVMPointerType(LLVMInt8Type(), 0)};
    LLVMAddFunction(module, "puts", LLVMFunctionType(LLVMInt8Type(), args, 1, 0));

    LLVMTypeRef i8_pointer_type = LLVMPointerType(LLVMInt8Type(), 0);
    LLVMTypeRef parameter_types[] = { i8_pointer_type, i8_pointer_type };

    /* Vector comparison operators */
    LLVMTypeRef function_type = LLVMFunctionType(LLVMInt8Type(), parameter_types, 2, 0);
    LLVMAddFunction(module, "op_v_eq_v", function_type);
    LLVMAddFunction(module, "op_v_neq_v", function_type);
    LLVMAddFunction(module, "op_v_lt_v", function_type);
    LLVMAddFunction(module, "op_v_gt_v", function_type);

    /* Vector Arithmetics operators */
    function_type = LLVMFunctionType(i8_pointer_type, parameter_types, 2, 0);
    LLVMAddFunction(module, "op_v_add_v", function_type);
    LLVMAddFunction(module, "op_v_sub_v", function_type);

    parameter_types[0] = LLVMDoubleType();
    function_type = LLVMFunctionType(i8_pointer_type, parameter_types, 2, 0);
    LLVMAddFunction(module, "op_s_mult_v", function_type);

    parameter_types[0] = i8_pointer_type;
    parameter_types[1] = LLVMDoubleType();
    function_type = LLVMFunctionType(LLVMDoubleType(), parameter_types, 2, 0);
    LLVMAddFunction(module, "op_v_pow_s", function_type);

    /* Integer comparison operator */
    parameter_types[0] = LLVMInt8Type();
    parameter_types[1] = LLVMInt8Type();
    function_type = LLVMFunctionType(LLVMInt8Type(), parameter_types, 2, 0);
    LLVMAddFunction(module, "op_i_eq_i", function_type);
}

LLVMModuleRef generate_module(struct node *ast, const char *name)
{
    LLVMModuleRef module = LLVMModuleCreateWithName(name);

    declare_external_methods(module);

    struct node *decl_seq = ast->childv[0];
    for (int i = 0; i < decl_seq->childc; i++) {
        struct node *decl = decl_seq->childv[i]->childv[0];
        struct payload *decl_payload = (struct payload *) decl->payload;
        if (decl_payload->type == N_RULE_DECLARATION) {
            generate_rule_declaration(module, decl);
        }
    }

    char *error = NULL;
    if (!LLVMVerifyModule(module, LLVMPrintMessageAction, &error)) {
        puts(error);
        free(error);
    }

    return module;
}
