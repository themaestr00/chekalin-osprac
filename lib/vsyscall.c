#include <inc/vsyscall.h>
#include <inc/lib.h>

static inline uint64_t
vsyscall(int num) {
    // LAB 12: Your code here
    int res;
    while ((res = vsys[num]) == 0)
        sys_yield();
    return res;
}

int
vsys_gettime(void) {
    return vsyscall(VSYS_gettime);
}
