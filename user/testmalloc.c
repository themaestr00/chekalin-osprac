#include <inc/lib.h>

void
umain(int argc, char **argv) {
    cprintf("Starting memory test...\n");

    // Test 1: Malloc and Free
    cprintf("Test 1: Malloc and Free... ");
    void *p1 = malloc(100);
    if (!p1) panic("malloc(100) failed");
    if (get_allocsize(p1) < 100) panic("get_allocsize(p1) < 100");
    memset(p1, 0xAA, 100);
    free(p1);
    cprintf("Passed\n");

    // Test 2: Calloc
    cprintf("Test 2: Calloc... ");
    int *p2 = calloc(10, sizeof(int));
    if (!p2) panic("calloc failed");
    for (int i = 0; i < 10; i++) {
        if (p2[i] != 0) panic("calloc didn't zero memory");
    }
    free(p2);
    cprintf("Passed\n");

    // Test 3: Realloc
    cprintf("Test 3: Realloc... ");
    char *p3 = malloc(10);
    if (!p3) panic("malloc(10) failed");
    strcpy(p3, "123456789");
    p3 = realloc(p3, 20);
    if (!p3) panic("realloc failed");
    if (strcmp(p3, "123456789") != 0) panic("realloc corrupted data");
    if (get_allocsize(p3) < 20) panic("realloc didn't resize correctly");
    free(p3);
    cprintf("Passed\n");

    // Test 4: Strdup
    cprintf("Test 4: Strdup... ");
    char *s = "hello world";
    char *d = strdup(s);
    if (!d) panic("strdup failed");
    if (strcmp(s, d) != 0) panic("strdup content mismatch");
    if (d == s) panic("strdup didn't copy");
    free(d);
    cprintf("Passed\n");

    // Test 5: Multiple allocations
    cprintf("Test 5: Multiple allocations... ");
    void *ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = malloc(1000);
        if (!ptrs[i]) panic("malloc failed in loop");
        memset(ptrs[i], i, 1000);
    }
    for (int i = 0; i < 10; i++) {
        char *buf = ptrs[i];
        for (int j = 0; j < 1000; j++) {
            if (buf[j] != (char)i) panic("memory corruption detected");
        }
        free(ptrs[i]);
    }
    cprintf("Passed\n");

    cprintf("All memory tests passed!\n");
}
