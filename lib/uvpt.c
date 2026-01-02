/* User virtual page table helpers */

#include <inc/lib.h>
#include <inc/mmu.h>

extern volatile pte_t uvpt[];     /* VA of "virtual page table" */
extern volatile pde_t uvpd[];     /* VA of current page directory */
extern volatile pdpe_t uvpdp[];   /* VA of current page directory pointer */
extern volatile pml4e_t uvpml4[]; /* VA of current page map level 4 */

pte_t
get_uvpt_entry(void *va) {
    if (!(uvpml4[VPML4(va)] & PTE_P)) return uvpml4[VPML4(va)];
    if (!(uvpdp[VPDP(va)] & PTE_P) || (uvpdp[VPDP(va)] & PTE_PS)) return uvpdp[VPDP(va)];
    if (!(uvpd[VPD(va)] & PTE_P) || (uvpd[VPD(va)] & PTE_PS)) return uvpd[VPD(va)];
    return uvpt[VPT(va)];
}

uintptr_t
get_phys_addr(void *va) {
    if (!(uvpml4[VPML4(va)] & PTE_P))
        return -1;
    if (!(uvpdp[VPDP(va)] & PTE_P))
        return -1;
    if (uvpdp[VPDP(va)] & PTE_PS)
        return PTE_ADDR(uvpdp[VPDP(va)]) + ((uintptr_t)va & ((1ULL << PDP_SHIFT) - 1));
    if (!(uvpd[VPD(va)] & PTE_P))
        return -1;
    if ((uvpd[VPD(va)] & PTE_PS))
        return PTE_ADDR(uvpd[VPD(va)]) + ((uintptr_t)va & ((1ULL << PD_SHIFT) - 1));
    if (!(uvpt[VPT(va)] & PTE_P))
        return -1;
    return PTE_ADDR(uvpt[VPT(va)]) + PAGE_OFFSET(va);
}

int
get_prot(void *va) {
    pte_t pte = get_uvpt_entry(va);
    int prot = pte & PTE_AVAIL & ~PTE_SHARE;
    if (pte & PTE_P) prot |= PROT_R;
    if (pte & PTE_W) prot |= PROT_W;
    if (!(pte & PTE_NX)) prot |= PROT_X;
    if (pte & PTE_SHARE) prot |= PROT_SHARE;
    return prot;
}

bool
is_page_dirty(void *va) {
    pte_t pte = get_uvpt_entry(va);
    return pte & PTE_D;
}

bool
is_page_present(void *va) {
    return get_uvpt_entry(va) & PTE_P;
}

int
foreach_shared_region(int (*fun)(void *start, void *end, void *arg), void *arg) {
    /* Calls fun() for every shared region.
     * NOTE: Skip over larger pages/page directories for efficiency */
    // LAB 11: Your code here:

    int res = 0;
    for (uintptr_t i = 0; i < 1; ++i) {
        if (!(uvpml4[i] & PTE_P)) continue;
        for (uintptr_t j = (i << PML4_ENTRY_SHIFT); j < (PDP_ENTRY_COUNT + (i << PML4_ENTRY_SHIFT)); ++j) {
            if (!(uvpdp[j] & PTE_P)) continue;
            if ((uvpdp[j] & PTE_PS) && (uvpdp[j] & PTE_SHARE)) {
                if ((j << PDP_SHIFT) < MAX_LOW_ADDR_KERN_SIZE) continue;
                res = fun((void *)(j << PDP_SHIFT), (void *)((j + 1) << PDP_SHIFT), arg);
                if (res < 0) return res;
            }
            if (!(uvpdp[j] & PTE_PS)) {
                for (uintptr_t k = (j << PDP_ENTRY_SHIFT); k < (PD_ENTRY_COUNT + (j << PDP_ENTRY_SHIFT)); ++k) {
                    if (!(uvpd[k] & PTE_P)) continue;
                    if ((uvpd[k] & PTE_PS) && (uvpd[k] & PTE_SHARE)) {
                        if ((k << PD_SHIFT) < MAX_LOW_ADDR_KERN_SIZE) continue;
                        res = fun((void *)(k << PD_SHIFT), (void *)((k + 1) << PD_SHIFT), arg);
                        if (res < 0) return res;
                    }
                    if (!(uvpd[k] & PTE_PS)) {
                        for (uintptr_t h = (k << PD_ENTRY_SHIFT); h < (PT_ENTRY_COUNT + (k << PD_ENTRY_SHIFT)); ++h) {
                            if ((h << PT_SHIFT) >= USER_STACK_TOP - USER_STACK_SIZE) break;
                            if ((h << PT_SHIFT) < MAX_LOW_ADDR_KERN_SIZE) continue;
                            if (uvpt[h] & PTE_P && uvpt[h] & PTE_SHARE) {
                                res = fun((void *)(h << PT_SHIFT), (void *)((h + 1) << PT_SHIFT), arg);
                                if (res < 0) return res;
                            }
                        }
                    }
                }
            }
        }
    }

    return res;
}
