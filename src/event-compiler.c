#include "codegen.h"
#include "compiler.h"
#include "validator.h"
#include "scope.h"
#include <stdio.h>

int main(int argc, const char *argv[])
{
    struct node *root = parse_ast("SampleEvent extends PositionEvent;PositionEvent extends BasicEvent;A: [p1, p2] -> a;B: [SampleEvent : p3, p4] -> foo;SampleEvent a() := {  pos = [1, 2, 3],  angle = [30.0]};SampleEvent foo(SampleEvent s1) := {  pos = 2 * s1.pos,  angle = [0]};predicate p1(SampleEvent s1, SampleEvent s2) := s1.pos == s2.pos;predicate p2(SampleEvent s1, SampleEvent s2) := s1.pos != s2.pos;predicate p3(SampleEvent s1, SampleEvent s2) := s1.pos < s2.pos;predicate p4(SampleEvent s1, SampleEvent s2) := s1.pos > s2.pos;predicate p5(SampleEvent s1, SampleEvent s2) := (s1.pos - s2.pos) < [1,1];");

    link_references(root);

    if (validate(root)) {
        printf("YAY \\o/\n");
    }

    LLVMModuleRef module = generate_module(root, "TestModule");

    tree_free(&root, payload_free);

    LLVMDumpModule(module);

    LLVMDisposeModule(module);

    return 0;
}
