#ifndef VGA_VGA_H
#define VGA_VGA_H

#include "pci/pci.h"
#include "pci/pci_classes.h"

#define VBE_DISPI_INDEX_ID          (0)
#define VBE_DISPI_INDEX_XRES        (1)
#define VBE_DISPI_INDEX_YRES        (2)
#define VBE_DISPI_INDEX_BPP         (3)
#define VBE_DISPI_INDEX_ENABLE      (4)
#define VBE_DISPI_INDEX_BANK        (5)
#define VBE_DISPI_INDEX_VIRT_WIDTH  (6)
#define VBE_DISPI_INDEX_VIRT_HEIGHT (7)
#define VBE_DISPI_INDEX_X_OFFSET    (8)
#define VBE_DISPI_INDEX_Y_OFFSET    (9)

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA  0x01CF

#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED  0x01

#define VBE_DISPI_BPP_4  (0x04)
#define VBE_DISPI_BPP_8  (0x08)
#define VBE_DISPI_BPP_15 (0x0F)
#define VBE_DISPI_BPP_16 (0x10)
#define VBE_DISPI_BPP_24 (0x18)
#define VBE_DISPI_BPP_32 (0x20)

#define VBE_DISPI_LFB_ENABLED (0x40)
#define VBE_DISPI_NOCLEARMEM  (0x80)

struct VideoGraphicsAdapter {
    struct PciDevice* pd;

    uintptr_t io_base;
    volatile uint32_t* fb_base_addr;
    volatile uint32_t* fb;

    uint32_t framebuffer_size;

    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t capabilities;
};

typedef struct VideoGraphicsAdapter VGA;

VGA* get_vga_instance(void);

int vga_fill_screen(uint32_t color_code);

int bochs_vbe_write_register(uint16_t index, uint16_t data);
int bochs_vbe_read_register(uint16_t index);

int bochs_vbe_init(uint16_t width, uint16_t height, uint16_t bpp);

#endif
