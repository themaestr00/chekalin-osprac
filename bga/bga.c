#include "bga.h"

#include "pci/pci.h"
#include "pci/pci_classes.h"

#include <inc/lib.h>

static VGA vga;

VGA *
get_vga_instance(void) {
    return &vga;
}

int
vga_fill_screen(uint32_t color_code) {
    cprintf("%d %d %lx\n", vga.height, vga.width, (uintptr_t)vga.fb);
    for (int i = 0; i < vga.height; ++i) {
        for (int j = 0; j < vga.width; ++j) {
            // cprintf("step by step\n");
            vga.fb[i * vga.width + j] = color_code;
        }
    }
    return 0;
}

int
bochs_vbe_write_register(uint16_t index, uint16_t data) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, data);
    return 0;
}

int
bochs_vbe_read_register(uint16_t index) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    return inw(VBE_DISPI_IOPORT_DATA);
}

int
bochs_vbe_init(uint16_t width, uint16_t height, uint16_t bpp) {
    struct PciDevice *pd = NULL;
    while (!(pd = find_pci_dev(PCI_CLASS_DISPLAY, 0))) {
        // cprintf("BGA: Waiting for PCI server...\n");
        sys_yield();
    }

    bochs_vbe_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    bochs_vbe_write_register(VBE_DISPI_INDEX_XRES, width);
    bochs_vbe_write_register(VBE_DISPI_INDEX_YRES, height);
    bochs_vbe_write_register(VBE_DISPI_INDEX_BPP, bpp);
    bochs_vbe_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
    vga.width = width;
    vga.height = height;
    vga.bpp = bpp;
    vga.pd = pd;

    vga.fb_base_addr = vga.fb = (volatile uint32_t *)VGA_FB_VADDR;

    size_t bar_size = get_bar_size(pd, 0);
    uintptr_t bar_addr = get_bar_address(pd, 0);

    cprintf("Bar size = %ld\n", bar_size);
    cprintf("Bar address = %lx\n", bar_addr);

    int res = sys_map_physical_region(bar_addr, 0, (void *)VGA_FB_VADDR, bar_size, PROT_RW);
    cprintf("res = %d\n", res);

#ifdef SANITIZE_USER_SHADOW_BASE
    platform_asan_unpoison((void *)VGA_FB_VADDR, bar_size);
#endif

    if (res)
        return -1;

    return 0;
}
