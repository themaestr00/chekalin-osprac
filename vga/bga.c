#include <inc/lib.h>
#include <pci/pci.h>
#include <pci/pci_classes.h>

#include "bga.h"

static BGA bga;
BGA *
bga_get_instance(void) {
    return &bga;
}

int
bga_init(unsigned int width, unsigned int height, unsigned int bpp) {
    struct PciDevice *pd = find_pci_dev(PCI_CLASS_DISPLAY, PCI_SUBCLASS_DISPLAY_VGA);

    bga_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    bga_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    bga_write_register(VBE_DISPI_INDEX_XRES, width);
    bga_write_register(VBE_DISPI_INDEX_YRES, height);
    bga_write_register(VBE_DISPI_INDEX_BPP, bpp);
    bga_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED | 0 /*VBE_DISPI_NOCLEARMEM*/);
    bga.width = width;
    bga.height = height;
    bga.bpp = bpp;
    bga.pd = pd;

    bga.fb_base_addr = bga.fb = (volatile uint32_t *)VGA_FB_VADDR;

    size_t bar_size = get_bar_size(pd, 0);
    uintptr_t bar_addr = get_bar_address(pd, 0);

    cprintf("Bar size = %ld\n", bar_size);
    cprintf("Bar address = %lx\n", bar_addr);

    int res = sys_map_physical_region(bar_addr, 0, (void *)VGA_FB_VADDR, bar_size, PROT_RW);
    cprintf("res = %d\n", res);
    uint32_t l = *((uint32_t *)(bga.fb + 0x20));
    cprintf("l = %d\n", l);

    return (res ? -1 : 0);
}

void
bga_write_register(uint16_t index_value, uint16_t data_value) {
    outpw(VBE_DISPI_IOPORT_INDEX, index_value);
    outpw(VBE_DISPI_IOPORT_DATA, data_value);
}

uint16_t
bga_read_register(uint16_t index_value) {
    outpw(VBE_DISPI_IOPORT_INDEX, index_value);
    return inpw(VBE_DISPI_IOPORT_DATA);
}

int
bga_is_available(void) {
    int vbe_id = bga_read_register(VBE_DISPI_INDEX_ID);
    for (size_t i = 0; i < BGA_NVERSIONS; i++) {
        if (vbe_id == bga_versions[i]) {
            return 1;
        }
    }
    return 0;
}

void
bga_set_video_mode(unsigned int width, unsigned int height, unsigned int bit_depth, int use_linear_frame_buffer, int clear_video_memory) {
    bga_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    bga_write_register(VBE_DISPI_INDEX_XRES, width);
    bga_write_register(VBE_DISPI_INDEX_YRES, height);
    bga_write_register(VBE_DISPI_INDEX_BPP, bit_depth);
    bga_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED |
                                                       (use_linear_frame_buffer ? VBE_DISPI_LFB_ENABLED : 0) |
                                                       (clear_video_memory ? 0 : VBE_DISPI_NOCLEARMEM));
}

void
bga_set_bank(uint16_t bank_number) {
    bga_write_register(VBE_DISPI_INDEX_BANK, bank_number);
}