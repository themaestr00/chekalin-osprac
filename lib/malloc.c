/*
 * User-level memory allocation routines
 */

#include <inc/lib.h>

static uintptr_t heap_top_ptr = USER_HEAP_TOP;
static size_t heap_page_ptr = -PAGE_SIZE;
static size_t heap_page_offset = PAGE_SIZE;
static MemoryDescriptor block_descr[MAX_MEM_DESCR];
static size_t descr_count = 0;
static size_t descr_count_free = 0;

void *
malloc(size_t nbytes) {
    int res;
    if (nbytes <= 0 || descr_count >= MAX_MEM_DESCR) {
        return NULL;
    }

    // Look at descriptors...
    if (descr_count_free) {
        for (size_t i = 0; i < descr_count; i++)
            if (block_descr[i].is_free && block_descr[i].size >= nbytes) {
                descr_count_free--;
                block_descr[i].is_free = false;
                block_descr[i].size = nbytes;
                return (void *)(block_descr[i].heap_ptr);
            }
    }
    // Not found.

    // Shall we alloc additional page?
    if (nbytes + heap_page_offset > PAGE_SIZE) {
        heap_page_ptr += PAGE_SIZE;
        heap_page_offset = 0;
        if ((res = sys_alloc_region(sys_getenvid(), (void *)(heap_top_ptr + heap_page_ptr), ROUNDUP(nbytes % PAGE_SIZE ? nbytes : nbytes + PAGE_SIZE, PAGE_SIZE),
                                    PROT_USER | PROT_R | PROT_W)) < 0) {
            return NULL;
        }
#ifdef SANITIZE_USER_SHADOW_BASE
        if (!res)
            platform_asan_unpoison((void *)(heap_top_ptr + heap_page_ptr), ROUNDUP(nbytes % PAGE_SIZE ? nbytes : nbytes + PAGE_SIZE, PAGE_SIZE));
#endif
    }
    nbytes = ROUNDUP(nbytes, sizeof(uintptr_t));
    block_descr[descr_count].heap_ptr = heap_top_ptr + heap_page_ptr + heap_page_offset;
    block_descr[descr_count].size = nbytes;
    block_descr[descr_count].is_free = false;
    heap_page_offset = (heap_page_offset + nbytes) % PAGE_SIZE;
    heap_page_ptr += nbytes - nbytes % PAGE_SIZE;

    return (void *)(block_descr[descr_count++].heap_ptr);
}

void *
calloc(size_t num, size_t size) {
    size_t dim = num * size;
    return memset(malloc(dim), 0, dim);
}

size_t
get_allocsize(void *ptr) {
    if (descr_count == descr_count_free)
        return 0;
    for (size_t i = 0; i < descr_count; i++)
        if ((uintptr_t)ptr == block_descr[i].heap_ptr)
            return block_descr[i].is_free ? 0 : block_descr[i].size;
    return 0;
}

void *
realloc(void *ptr, size_t newsize) {
    size_t oldsize = get_allocsize(ptr);
    if (oldsize > newsize)
        return ptr;
    void *newptr = memcpy(malloc(newsize), ptr, oldsize);
    free(ptr);
    return newptr;
}

void
free(void *ptr) {
    if (!ptr)
        return;

    if (descr_count == descr_count_free)
        panic("trying to free unallocated memory.\n");

    for (size_t i = 0; i < descr_count; i++) {
        if ((uintptr_t)ptr == block_descr[i].heap_ptr) {
            if (block_descr[i].is_free)
                panic("trying to free already freed memory.\n");
            block_descr[i].is_free = true;
            descr_count_free++;
            return;
        }
    }

    panic("trying to free invalid memory pointer.\n");
}

char *
strdup(const char *src) {
    char *str;
    char *p;
    int len = 0;

    while (src[len])
        len++;
    str = malloc(len + 1);
    p = str;
    while (*src)
        *p++ = *src++;
    *p = '\0';
    return str;
}
