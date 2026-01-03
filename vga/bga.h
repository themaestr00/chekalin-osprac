#pragma once

#include <inc/lib.h>

const static uint32_t bga_versions[] = {
        0xB0C0, 0xB0C1, 0xB0C2, 0xB0C3, 0xB0C4, 0xB0C5};

#define BGA_NVERSIONS (sizeof(bga_versions) / sizeof(bga_versions[0]))

struct BochsGraphicsAdapter {
    struct PciDevice *pd;
    uintptr_t io_base;

    // Framebuffer info
    volatile uint32_t *fb_base_addr;
    volatile uint32_t *fb;
    uint32_t framebuffer_size;

    // Current mode info
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t capabilities;
};

typedef struct BochsGraphicsAdapter BGA;

// VBE DISPI mode
#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED  0x01

// Indexes for Bochs VBE DISPI registers
#define VBE_DISPI_INDEX_ID          0
#define VBE_DISPI_INDEX_XRES        1
#define VBE_DISPI_INDEX_YRES        2
#define VBE_DISPI_INDEX_BPP         3
#define VBE_DISPI_INDEX_ENABLE      4
#define VBE_DISPI_INDEX_BANK        5
#define VBE_DISPI_INDEX_VIRT_WIDTH  6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 7
#define VBE_DISPI_INDEX_X_OFFSET    8
#define VBE_DISPI_INDEX_Y_OFFSET    9

// Values for the VBE DISPI ENABLE register
#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA  0x01CF

// BPP values
#define VBE_DISPI_BPP_4  0x04
#define VBE_DISPI_BPP_8  0x08
#define VBE_DISPI_BPP_15 0x0F
#define VBE_DISPI_BPP_16 0x10
#define VBE_DISPI_BPP_24 0x18
#define VBE_DISPI_BPP_32 0x20

// Bank mode stuff
#define VBE_DISPI_BANK_SIZE_KB 64
#define VBE_DISPI_BANK_ADDRESS 0xA0000

// Linear framebuffer and memory clear flags
#define VBE_DISPI_LFB_ENABLED 0x40
#define VBE_DISPI_NOCLEARMEM  0x80

BGA *bga_get_instance(void);
int bga_init(unsigned int width, unsigned int height, unsigned int bpp);
void bga_write_register(uint16_t index_value, uint16_t data_value);
uint16_t bga_read_register(uint16_t index_value);
int bga_is_available(void);
void bga_set_video_mode(unsigned int width, unsigned int height, unsigned int bit_depth, int use_linear_frame_buffer, int clear_video_memory);
void bga_set_bank(uint16_t bank_number);