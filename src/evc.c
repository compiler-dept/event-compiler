#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <llvm-c/BitWriter.h>
#include "string.h"
#include "codegen.h"
#include "compiler.h"
#include "validator.h"
#include "scope.h"
#include "astdump.h"

struct flags {
    int input;
    char input_path[255];
    int output;
    char output_path[255];
    int validation;
    int code_generation;
    int astdump;
    char astdump_path[255];
};

const char *usage = "\
The Event Compiler\n\n\
Usage: evc [options] -i <file>\n\n\
OPTIONS:\n\
  -i <file>             Read input from <file>\n\
  -o <file>             Write output to <file>\n\
  -a <file>             Write ast to <file>\n\
  -h                    Show this help\n\
  -v                    Show Event Compiler version\n\
  -V                    No validation\n\
  -C                    No code generation\
";

const size_t BUFFER_SIZE = 1024;

int main(int argc, char *const *argv)
{
    struct flags flags;
    flags.input = 0;
    flags.output = 0;
    flags.astdump = 0;
    flags.validation = 1;
    flags.code_generation = 1;

    int ch;
    while ((ch = getopt(argc, argv, "Chi:o:a:vV")) != -1) {
        switch (ch) {
            case 'i':
                flags.input = 1;
                strcpy(flags.input_path, optarg);
                break;
            case 'o':
                flags.output = 1;
                strcpy(flags.output_path, optarg);
                break;
            case 'a':
                flags.astdump = 1;
                strcpy(flags.astdump_path, optarg);
                break;
            case 'h':
                fprintf(stdout, "%s\n", usage);
                return EXIT_SUCCESS;
                break;
            case 'v':
                fprintf(stdout, "%s\n", "The Event Compiler 0.1.0");
                return EXIT_SUCCESS;
                break;
            case 'V':
                flags.validation = 0;
                break;
            case 'C':
                flags.code_generation = 0;
                break;
            default:
                return EXIT_FAILURE;
        }
    }

    if (flags.input == 0) {
        fprintf(stderr, "%s\n", usage);
        return EXIT_FAILURE;
    }

    if (access(flags.input_path, F_OK) == -1) {
        perror(flags.input_path);
        return EXIT_FAILURE;
    }

    FILE *input_file = fopen(flags.input_path, "r");
    if (!input_file) {
        perror(flags.input_path);
        return EXIT_FAILURE;
    }

    char *input = malloc(BUFFER_SIZE * sizeof(char));
    size_t size = BUFFER_SIZE;
    int offset = 0;

    size_t r = 0;
    while ((r = fread(input + offset, sizeof(char), BUFFER_SIZE, input_file)) != 0) {
        if (r == BUFFER_SIZE) {
            size += BUFFER_SIZE;
            input = realloc(input, size * sizeof(char));
        }

        offset += r;
        input[offset] = '\0';
    }

    fclose(input_file);

    struct node *root = parse_ast(input);
    if (!root) {
        free(input);
        return EXIT_FAILURE;
    }

    link_references(root);

    if (flags.validation) {
        if (!validate(root)) {
            puts("Validation failed.");
            tree_free(&root, payload_free);
            free(input);
            return EXIT_FAILURE;
        }
    }

    if (flags.code_generation) {
        LLVMModuleRef module = generate_module(root, flags.input_path);
        if (flags.output) {
            LLVMWriteBitcodeToFile(module, flags.output_path);
        } else {
            LLVMDumpModule(module);
        }
        LLVMDisposeModule(module);
    }

    if (flags.astdump){
        dump_ast(root, flags.astdump_path);
    }

    tree_free(&root, payload_free);

    free(input);

    return EXIT_SUCCESS;
}
