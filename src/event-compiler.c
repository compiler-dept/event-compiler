#include <stdio.h>
#include "codegen.h"
#include "compiler.h"
#include "validator.h"
#include "scope.h"

int main(int argc, const char **argv)
{
    struct node *root = parse_ast("\
        event SampleEvent { pos, time };\
        event EnheritanceEvent extends SampleEvent { angle };\
        \
        predicate sample_prediacte(SampleEvent sample_a, SampleEvent sample_b) := sample_a.pos == sample_b.pos;\
        \
        PosEqual: [SampleEvent : sample_predicate] -> example;\
        \
        /*SampleEvent example(SampleEvent sampleEvent) := {  pos = 3 * sampleEvent.pos, time = [12.3] };*/\
    ");

    link_references(root);

    if (validate(root)) {
        printf("YAY \\o/\n");
    } else {
        puts("Validation failed.");
    }

    /*
    LLVMModuleRef module = generate_module(root, "TestModule");

    tree_free(&root, payload_free);

    LLVMDumpModule(module);

    LLVMDisposeModule(module);
    */

    tree_free(&root, payload_free);

    return 0;
}
