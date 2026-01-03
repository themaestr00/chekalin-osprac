#include "pci/pci.h"
#include <inc/string.h>
#include <inc/lib.h>

union PciIpc pciipcbuf __attribute__((aligned(PAGE_SIZE)));

uint64_t tsc_freq;

static int
pciipc(unsigned type, void *dstva) {
    static envid_t pcienv;

    if (!pcienv) pcienv = ipc_find_env(ENV_TYPE_PCI);

    static_assert(sizeof(pciipcbuf) == PAGE_SIZE, "Invalid pciipcbuf size");

    if (1) {
        cprintf("[%08x] pciipc %d %08x\n",
                thisenv->env_id, type, *(uint32_t *)&pciipcbuf);
    }

    ipc_send(pcienv, type, &pciipcbuf, PAGE_SIZE, PROT_RW);
    size_t maxsz = PAGE_SIZE;
    return ipc_recv_from(NULL, dstva, &maxsz, NULL, pcienv);
}

struct PciDevice *
find_pci_dev(int class, int sub) {
    pciipcbuf.find.class = class;
    pciipcbuf.find.sub = sub;

    int res = pciipc(PCIREQ_FIND_PCI_DEV, NULL);
    if (res < 0) return NULL;

    return pciipcbuf.findRet.pcid;
}

uint32_t
get_bar_size(struct PciDevice *pcid, uint32_t barno) {
    pciipcbuf.barsize.pcid = pcid;
    pciipcbuf.barsize.barno = barno;

    int res = pciipc(PCIREQ_GET_BAR_SIZE, NULL);
    if (res < 0) return 0;

    return pciipcbuf.barsizeRet.size;
}

uintptr_t
get_bar_address(struct PciDevice *pcid, uint32_t barno) {
    pciipcbuf.barpaddr.pcid = pcid;
    pciipcbuf.barpaddr.barno = barno;

    int res = pciipc(PCIREQ_GET_BAR_ADDRESS, NULL);
    if (res < 0) return 0;

    return pciipcbuf.barpaddrRet.address;
}

uintptr_t
get_tsc_freq() {
    int res = pciipc(PCIREQ_GET_TSC_FREQ, NULL);
    if (res < 0) return 0;

    return pciipcbuf.tscfreqRet.tsc_freq;
}