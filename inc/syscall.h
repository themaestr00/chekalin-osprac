#ifndef JOS_INC_SYSCALL_H
#define JOS_INC_SYSCALL_H

#define MAX_MEM_DESCR 0x10000

#include <inc/types.h>

typedef struct {
    uintptr_t heap_ptr;
    size_t size;
    bool is_free;
} MemoryDescriptor;

/* system call numbers */
enum {
    SYS_cputs = 0,
    SYS_cgetc,
    SYS_getenvid,
    SYS_env_destroy,
    SYS_alloc_region,
    SYS_map_region,
    SYS_map_physical_region,
    SYS_unmap_region,
    SYS_region_refs,
    SYS_exofork,
    SYS_env_set_status,
    SYS_env_set_trapframe,
    SYS_env_set_pgfault_upcall,
    SYS_yield,
    SYS_ipc_try_send,
    SYS_ipc_recv,
    SYS_ipc_recv_from,
    SYS_gettime,
    SYS_resize_display,
    SYS_display_change_vga_state,
    NSYSCALLS
};

#endif /* !JOS_INC_SYSCALL_H */
