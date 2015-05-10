#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Target.h>
#include <hashmap.h>
#include "codegen.h"


struct hashmap *function_arguments = NULL;

void generate_expression(LLVMModuleRef module, LLVMBuilderRef builder,
                         LLVMValueRef target_value, struct node *node);

void generate_additive_expression(LLVMModuleRef module, LLVMBuilderRef builder,
                                  LLVMValueRef target_value, struct node *node);

void generate_multiplicative_expression(LLVMModuleRef module, LLVMBuilderRef builder,
                                        LLVMValueRef target_value, struct node *node);

int generate_event_fields(struct stack **members, struct node *node)
{
    int member_count = node->childv[0]->childc;

    for (int i = 0; i < member_count; i++) {
        stack_push(members, LLVMPointerType(LLVMDoubleType(), 0));
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
        member_types[i] = stack_pop(&members);
        member_types[i + 1] = stack_pop(&members);
    }

    LLVMStructSetBody(event_type, member_types, 2 * member_count, 1);
    return event_type;
}

LLVMTypeRef generateEventTypeIfNecessary(LLVMModuleRef module, struct node *event_decl)
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


void generate_rule_declaration(LLVMModuleRef module, struct node *node)
{
    struct payload *payload = node->payload;
    char active_name[1 + 7 + strlen(payload->rule_declaration.name)];
    char function_name[1 + 9 + strlen(payload->rule_declaration.name)];

    sprintf(active_name, "%s_active", payload->rule_declaration.name);
    sprintf(function_name, "%s_function", payload->rule_declaration.name);

    if (((struct payload *)node->childv[0]->payload)->alternative == ALT_PREDICATE_SEQUENCE) {
        struct node *event_sequence = node->childv[0]->childv[0];
        LLVMTypeRef parameters[event_sequence->childc];
        for (int i = 0; i < event_sequence->childc; i++) {
            struct node *event = event_sequence->childv[i];
            struct payload *event_payload = (struct payload *) event->payload;
            parameters[i] = LLVMPointerType(generateEventTypeIfNecessary(module, event_payload->event.ref), 0);
        }
        LLVMAddFunction(module, active_name,
                        LLVMFunctionType(LLVMInt8Type(), parameters, event_sequence->childc, 0));

        struct payload *func_payload = ((struct payload *)payload->rule_declaration.ref->payload);
        struct node *event = func_payload->function_definition.event_ref;

        LLVMTypeRef return_type = LLVMPointerType(generateEventTypeIfNecessary(module, event), 0);

        LLVMAddFunction(module, function_name,
                        LLVMFunctionType(return_type, parameters, event_sequence->childc, 0));
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
        LLVMTypeRef event_type = generateEventTypeIfNecessary(module, event);
        LLVMTypeRef parameter_type = LLVMPointerType(event_type, 0);
        (*parameters)[i] = parameter_type;
    }

    return parameter_count;
}



void generate_initializer(LLVMModuleRef module, LLVMBuilderRef builder,
                          LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    int index_in_struct = 2 * payload->initializer.ref_index;
    LLVMValueRef vector_in_struct = LLVMBuildStructGEP(builder, target_value, index_in_struct, "");
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
    struct node *expression_sequence = node->childv[0]->childv[0];
    int count = expression_sequence->childc;

    LLVMBuildStore(builder, LLVMConstInt(LLVMInt16Type(), count, 0), target_value);
    LLVMValueRef indices[2];
    indices[0] = LLVMConstInt(LLVMInt16Type(), 1, 0);
    LLVMValueRef _vector = LLVMBuildGEP(builder, target_value, indices, 1, "");
    LLVMTypeRef dest_type = LLVMPointerType(LLVMDoubleType(), 0);
    dest_type = LLVMPointerType(dest_type, 0);
    LLVMValueRef vector = LLVMBuildBitCast(builder, _vector, dest_type, "");

    LLVMTypeRef array_type = LLVMArrayType(LLVMDoubleType(), count);
    LLVMValueRef array = LLVMBuildMalloc(builder, array_type, "");

    generate_component_sequence(module, builder, array, node->childv[0]);

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
    if (payload->atomic.identifier[1] == NULL) {
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
        puts("ERROR NOT IMPLEMENTED YET");
    }
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

void generate_negation(LLVMModuleRef module, LLVMBuilderRef builder,
                       LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    if (payload->alternative  == ALT_PRIMARY_EXPRESSION) {
        generate_primary_expression(module, builder, target_value, node->childv[0]);
    } else if (payload->alternative  == ALT_NEGATION) {
        LLVMValueRef value_ptr = LLVMBuildAlloca(builder, LLVMDoubleType(), "");
        generate_negation(module, builder, value_ptr, node->childv[0]);
        LLVMValueRef value = LLVMBuildLoad(builder, value_ptr, "");
        LLVMValueRef neg_one = LLVMConstReal(LLVMDoubleType(), -1);
        LLVMValueRef result = LLVMBuildFMul(builder, neg_one, value, "");
        LLVMBuildStore(builder, result, target_value);
    }
}

void generate_multiplication(LLVMModuleRef module, LLVMBuilderRef builder,
                             LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    if (LLVMGetElementType(LLVMTypeOf(target_value)) == LLVMDoubleType()) {
        LLVMValueRef left_value_ptr = LLVMBuildAlloca(builder, LLVMDoubleType(), "");
        generate_multiplicative_expression(module, builder, left_value_ptr, node->childv[0] );
        LLVMValueRef left_value = LLVMBuildLoad(builder, left_value_ptr, "");

        LLVMValueRef right_value_ptr = LLVMBuildAlloca(builder, LLVMDoubleType(), "");
        generate_negation(module, builder, right_value_ptr, node->childv[1] );
        LLVMValueRef right_value = LLVMBuildLoad(builder, right_value_ptr, "");

        LLVMValueRef result;
        if (payload->alternative == ALT_MULT) {
            result = LLVMBuildFMul(builder, left_value, right_value, "");
        } else {
            result = LLVMBuildFDiv(builder, left_value, right_value, "");
        }
        LLVMBuildStore(builder, result, target_value);
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



void generate_addition(LLVMModuleRef module, LLVMBuilderRef builder,
                       LLVMValueRef target_value, struct node *node)
{
    struct payload *payload = node->payload;
    if (LLVMGetElementType(LLVMTypeOf(target_value)) == LLVMDoubleType()) {
        LLVMValueRef left_value_ptr = LLVMBuildAlloca(builder, LLVMDoubleType(), "");
        generate_additive_expression(module, builder, left_value_ptr, node->childv[0] );
        LLVMValueRef left_value = LLVMBuildLoad(builder, left_value_ptr, "");

        LLVMValueRef right_value_ptr = LLVMBuildAlloca(builder, LLVMDoubleType(), "");
        generate_multiplicative_expression(module, builder, right_value_ptr, node->childv[1] );
        LLVMValueRef right_value = LLVMBuildLoad(builder, right_value_ptr, "");

        LLVMValueRef result;
        if (payload->alternative == ALT_ADD) {
            result = LLVMBuildFAdd(builder, left_value, right_value, "");
        } else {
            result = LLVMBuildFSub(builder, left_value, right_value, "");
        }
        LLVMBuildStore(builder, result, target_value);
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
    switch (payload->alternative) {
        case ALT_ADDITIVE_EXPRESSION:
            generate_additive_expression(module, builder, target_value, node->childv[0]);
            break;
        default:
            puts("ERROR NOT IMPLEMENTED YET");
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

    LLVMTypeRef return_event_type = generateEventTypeIfNecessary(module,
                                    payload->function_definition.event_ref);
    LLVMTypeRef return_type = LLVMPointerType(return_event_type, 0);

    LLVMTypeRef function_type;
    int parameter_count = 0;


    if (payload->alternative == ALT_PARAMETER_LIST) {
        LLVMTypeRef *parameters = NULL;
        parameter_count = generate_parameter_list(module, node->childv[0], &parameters);
        function_type = LLVMFunctionType(return_type, parameters, parameter_count, 0);
        free(parameters);
    } else {
        function_type = LLVMFunctionType(return_type, NULL, 0, 0);
    }

    LLVMValueRef function = LLVMAddFunction(module, payload->function_definition.identifier,
                                            function_type);

    LLVMValueRef arg_values[parameter_count];
    LLVMGetParams(function, arg_values);

    for (int i = 0; i < parameter_count; i++) {
        struct payload *parameter_payload = node->childv[0]->childv[i]->payload;
        char *key = parameter_payload->parameter.identifier;
        hashmap_put(&function_arguments, key, arg_values[i]);
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
    }

    LLVMValueRef return_val = LLVMBuildLoad(builder, return_ptr, "");
    LLVMBuildRet(builder, return_val);
    LLVMDisposeBuilder(builder);
}


LLVMModuleRef generate_module(struct node *ast, const char *name)
{
    LLVMModuleRef module = LLVMModuleCreateWithName(name);

    struct tree_iterator *it = tree_iterator_init(&ast, PREORDER);
    struct node *temp = NULL;
    struct payload *payload = NULL;
    int success = 1;

    while ((temp = tree_iterator_next(it)) != NULL) {
        payload = temp->payload;
        switch (payload->type) {
            case N_RULE_DECLARATION:
                generate_rule_declaration(module, temp);
                break;
            case N_FUNCTION_DEFINITION:
                generate_function_definition(module, temp);
                break;
            default:
                break;
        }
        if (!success) {
            break;
        }
    }

    tree_iterator_free(it);

    char *foo = NULL;
    if (!LLVMVerifyModule(module, LLVMPrintMessageAction, &foo)) {
        puts(foo);
        free(foo);
    }

    return module;
}
