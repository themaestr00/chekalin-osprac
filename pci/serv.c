#include <inc/x86.h>
#include <inc/string.h>
#include <inc/lib.h>

#include "pci/pci.h"

union PciIpc *pcireq = (union PciIpc *)0x0FFF6000;

typedef int (*pcihandler)(envid_t envid, union PciIpc *req);

extern struct PciDevice pci_device_buffer[PCI_MAX_DEVICES];

extern struct PcieIoOps pcie_io;

int
serv_find_pci_dev(envid_t envid, union PciIpc *req) {
    int class = req->find.class;
    int sub = req->find.sub;
    for (int i = 0; i != PCI_MAX_DEVICES; i++) {
        if (pci_device_buffer[i].class == class &&
            pci_device_buffer[i].subclass == sub) {
            DEBUG("Found PCI device: class %X, subclass %X\n", class, sub);
            req->findRet.pcid = &pci_device_buffer[i];
            return 0;
        }
    }
    return -1;
}

int
serv_get_bar_size(envid_t envid, union PciIpc *req) {
    uint32_t barno = req->barsize.barno;
    if (barno >= PCI_BAR_COUNT)
        return -1;

    uint32_t tmp = req->barsize.pcid->bars[barno].base_address;

    /* Overwrite BARx address and read it back to get its size */
    pcie_io.write32(req->barsize.pcid, PCI_REG_BAR0 + 4 * barno, 0xFFFFFFFF);

    uint32_t outv = pcie_io.read32(req->barsize.pcid, PCI_REG_BAR0 + 4 * barno) & PCI_BAR_MEMORY_MASK;
    uint32_t size = ~outv + 1;

    /* Restore previous BARn value */
    pcie_io.write32(req->barsize.pcid, PCI_REG_BAR0 + 4 * barno, tmp);
    DEBUG("BAR%d: umnasked val = %x, size = 0x%x\n", barno, outv, size);

    req->barsizeRet.size = size;

    return 0;
}

int
serv_get_bar_address(envid_t envid, union PciIpc *req) {
    if (req->barpaddr.barno >= PCI_BAR_COUNT)
        return -1;

    uintptr_t base_addr = req->barpaddr.pcid->bars[0].base_address;
    if (req->barpaddr.pcid->bars[0].address_is_64bits)
        base_addr |= (uint64_t)(pcie_io.read32(req->barpaddr.pcid, PCI_REG_BAR0 + 4)) << 32;

    req->barpaddrRet.address = base_addr;

    return 0;
}

int
serv_get_tsc_freq(envid_t envid, union PciIpc *req) {
    req->tscfreqRet.tsc_freq = tsc_freq;
    return 0;
}

pcihandler handlers[] = {
        [PCIREQ_FIND_PCI_DEV] = serv_find_pci_dev,
        [PCIREQ_GET_BAR_SIZE] = serv_get_bar_size,
        [PCIREQ_GET_BAR_ADDRESS] = serv_get_bar_address,
        [PCIREQ_GET_TSC_FREQ] = serv_get_tsc_freq};
#define NHANDLERS (sizeof(handlers) / sizeof(handlers[0]))

void
serve(void) {
    uint32_t req, whom;
    int perm, res;
    void *pg;

    while (1) {
        perm = 0;
        size_t sz = PAGE_SIZE;
        req = ipc_recv((int32_t *)&whom, pcireq, &sz, &perm);
        if (debug) {
            cprintf("pci req %d from %08x [page %08lx: %s]\n",
                    req, whom, (unsigned long)get_uvpt_entry(pcireq),
                    (char *)pcireq);
        }

        enum EnvType env_type = ipc_find_env_type(whom);
        if (env_type == ENV_TYPE_USER) {
            ipc_send(whom, -E_BAD_ENV, NULL, 0, perm);
        } else {
            /* All requests must contain an argument page */
            if (!(perm & PROT_R)) {
                cprintf("Invalid request from %08x: no argument page\n", whom);
                continue; /* Just leave it hanging... */
            }

            pg = NULL;
            if (req < NHANDLERS && handlers[req]) {
                res = handlers[req](whom, pcireq);
            } else {
                cprintf("Invalid request code %d from %08x\n", req, whom);
                res = -E_INVAL;
            }
            ipc_send(whom, res, pg, PAGE_SIZE, perm);
        }
        sys_unmap_region(0, pcireq, PAGE_SIZE);
    }
}

void
umain(int argc, char **argv) {
    binaryname = "pci";
    cprintf("PCI Server is initializing\n");

    pci_init(argv);

    cprintf("PCI Server is running\n");

    serve();
}