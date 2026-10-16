/* Chapter 16: use the IR as a compiler client. C11; link pebbleir.c. */
#include "pebbleir.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fputs("usage: pebble 'expression using x, bytes, + and *'\n", stderr);
        return EXIT_FAILURE;
    }
    Pir f; PirVm vm; char error[PIR_ERROR];
    if (!pir_compile(argv[1], &f, error) || !pir_optimize(&f, error) ||
        !pir_lower(&f, &vm, error)) {
        fprintf(stderr, "%s\n", error); return EXIT_FAILURE;
    }
    if (!pir_print(stdout, &f)) return EXIT_FAILURE;
    PirResult r = pir_vm_run(&vm, 9, 10000);
    if (r.status != PIR_OK) return EXIT_FAILURE;
    fprintf(stderr, "x=9 result=%u vm-dispatches=%u\n", r.value, r.steps);
    return EXIT_SUCCESS;
}
