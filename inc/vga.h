#ifndef INC_VGA_H
#define INC_VGA_H

#include <inc/types.h>
#include <inc/mmu.h>

typedef enum vga_command {
    VGAREQ_WINDOW_CREATE = 1,
    VGAREQ_WINDOW_DESTROY,
    VGAREQ_RENDERER_CREATE,
    VGAREQ_RENDERER_DESTROY,
    VGAREQ_TEXTURE_CREATE,
    VGAREQ_TEXTURE_DESTROY,
    VGAREQ_TEXTURE_UPDATE,
    VGAREQ_TEXTURE_COPY,
    VGAREQ_DISPLAY,
    VGAREQ_CLEAR_RENDERER,
} vga_command;

typedef enum vga_window_mode{
    VGA_WINDOW_MODE_CORNER = 0,
    VGA_WINDOW_MODE_FULLSCREEN,
    VGA_WINDOW_MODE_CENTERED,
    VGA_WINDOW_MODE_SCALED_2X
} vga_window_mode;

struct Window {
    uint32_t width;
    uint32_t height;
    uint32_t x_offset;
    uint32_t y_offset;
    vga_window_mode mode;
};
typedef struct Window * Window_ptr;

struct Texture {
    uint32_t width;
    uint32_t height;
    uint32_t *buf;
    uint32_t *mapped_user_buf;
};
typedef struct Texture * Texture_ptr;

struct Renderer {
    struct Window *window;
    uint32_t *back_buffer;
};
typedef struct Renderer * Renderer_ptr;

union Vgaipc {
    struct Vgareq_window_create {
        uint32_t width;
        uint32_t height;
        vga_window_mode mode;
    } window_create;
    struct Vgaret_window_create_ret {
        Window_ptr window;
    } window_create_ret;
    struct Vgareq_window_destroy {
        Window_ptr window;
    } destroy_window;
    struct Vgareq_renderer_create {
        Window_ptr window;
    } renderer_create;
    struct Vgareq_renderer_create_ret {
        Renderer_ptr renderer;
    } renderer_create_ret;
    struct Vgareq_renderer_destroy {
        Renderer_ptr renderer;
    } renderer_destroy;
    struct Vgareq_texture_create {
        uint32_t width;
        uint32_t height;
        bool need_mapping;
    } texture_create;
    struct Vgareq_texture_create_ret {
        Texture_ptr texture;
        uint32_t *buffer_map;
    } texture_create_ret;
    struct Vgareq_texture_destroy {
        Texture_ptr texture;
    } texture_destroy;
    struct Vgareq_texture_update {
        Texture_ptr texture;
        size_t offset;
        size_t size;
        char buf[PAGE_SIZE - sizeof (size_t) * 3];
    } texture_update;
    struct Vgareq_texture_copy {
        Texture_ptr texture;
        Renderer_ptr renderer;
    } texture_copy;
    struct Vgareq_display {
        Renderer_ptr renderer;
    } display;
    struct Vgareq_clear_renderer {
        Renderer_ptr renderer;
    } clear_renderer;
    char _pad[PAGE_SIZE];
};

#endif
