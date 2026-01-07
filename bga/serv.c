#include <inc/x86.h>
#include <inc/string.h>
#include <inc/vga.h>
#include <inc/lib.h>

#include "bga.h"

union Vgaipc *vsreq = (union Vgaipc *)0x0FFFF000;

int
command_window_create(envid_t source, union Vgaipc *ipc) {
    VGA *vga_instance = get_vga_instance();
    struct Vgareq_window_create *command = &ipc->window_create;
    struct Window *window = (struct Window *)malloc(sizeof(struct Window));
    if (!window) {
        return -1;
    }
    window->width = command->width;
    window->height = command->height;
    window->mode = command->mode;
    ipc->window_create_ret.window = window;

    if (window->mode == VGA_WINDOW_MODE_CENTERED) {
        window->x_offset = vga_instance->width / 2 - window->width / 2;
        window->y_offset = vga_instance->height / 2 - window->height / 2;
    }
    vga_fill_screen(0x00000000);
    return 0;
}

int
command_window_destroy(envid_t source, union Vgaipc *ipc) {
    free(ipc->destroy_window.window);
    return 0;
}

int
command_renderer_create(envid_t source, union Vgaipc *ipc) {
    struct Renderer *renderer = (struct Renderer *)malloc(sizeof(struct Renderer));
    struct Vgareq_renderer_create *command = &ipc->renderer_create;
    if (!renderer) {
        return -1;
    }
    renderer->window = command->window;
    renderer->back_buffer = malloc(sizeof(uint32_t) * renderer->window->width * renderer->window->height);
    if (!renderer->back_buffer) {
        free(renderer);
        return -1;
    }
    ipc->renderer_create_ret.renderer = renderer;
    return 0;
}

int
command_renderer_destroy(envid_t source, union Vgaipc *ipc) {
    free(ipc->renderer_destroy.renderer->back_buffer);
    free(ipc->renderer_destroy.renderer);
    return 0;
}

int
command_texture_create(envid_t source, union Vgaipc *ipc) {
    int res;
    struct Texture *texture = (struct Texture *)malloc(sizeof(struct Texture));
    struct Vgareq_texture_create *command = &ipc->texture_create;
    if (!texture) {
        if (debug) cprintf("Malloc failed\n");
        return -1;
    }
    texture->width = command->width;
    texture->height = command->height;
    size_t tex_bytes = sizeof(uint32_t) * texture->width * texture->height;
    texture->buf = malloc(tex_bytes);
    if (!texture->buf) {
        if (debug) cprintf("Malloc 2 failed\n");
        free(texture);
        return -1;
    }
    ipc->texture_create_ret.texture = texture;
    if (command->need_mapping) {
        uint32_t *texture_map = (uint32_t *)(VIDEO_MAP_TOP + ((char *)texture->buf - USER_HEAP_TOP));
        size_t map_bytes = ROUNDUP(tex_bytes, PAGE_SIZE);
        if ((res = sys_map_region(0, texture->buf, source, texture_map, map_bytes, PROT_RW))) {
            if (debug) cprintf("Bad sys map %d\n", res);
            free(texture->buf);
            free(texture);
            return -res;
        }
        texture->mapped_user_buf = texture_map;
        ipc->texture_create_ret.buffer_map = texture_map;
    }
    return 0;
}

int
command_texture_destroy(envid_t source, union Vgaipc *ipc) {
    int res;
    struct Texture *texture = ipc->texture_destroy.texture;
    if (texture->mapped_user_buf) {
        size_t tex_bytes = sizeof(uint32_t) * texture->width * texture->height;
        size_t map_bytes = ROUNDUP(tex_bytes, PAGE_SIZE);
        if ((res = sys_unmap_region(source, texture->mapped_user_buf, map_bytes))) {
            return res;
        }
    }
    free(ipc->texture_destroy.texture->buf);
    free(ipc->texture_destroy.texture);
    return 0;
}

int
command_texture_update(envid_t source, union Vgaipc *ipc) {
    struct Vgareq_texture_update *command = &ipc->texture_update;
    struct Texture *texture = command->texture;
    memcpy((char *)texture->buf + command->offset, command->buf, command->size);
    return 0;
}

int
command_texture_copy(envid_t source, union Vgaipc *ipc) {
    struct Texture *texture = ipc->texture_copy.texture;
    struct Renderer *renderer = ipc->texture_copy.renderer;
    if (debug) cprintf("This code will be executed with error...\n");
    if (debug) cprintf("%lx %lx \n", (uintptr_t)renderer->back_buffer, (uintptr_t)texture->buf);
    memcpy(renderer->back_buffer, texture->buf, renderer->window->width * renderer->window->height * sizeof(uint32_t));
    if (debug) cprintf("...or not?\n");
    return 0;
}

int
command_display(envid_t source, union Vgaipc *ipc) {
    VGA *vga_instance = get_vga_instance();
    struct Renderer *renderer = ipc->display.renderer;
    struct Window *window = renderer->window;
    switch (window->mode) {
    case VGA_WINDOW_MODE_FULLSCREEN:
        for (uint32_t y = 0; y < vga_instance->height; ++y) {
            uint32_t new_y = (uint64_t)((y *  window->height) / vga_instance->height);
            for (uint32_t x = 0; x < vga_instance->width; ++x) {
                uint32_t new_x = (uint64_t)((x * window->width) / vga_instance->width);
                vga_instance->fb[y * vga_instance->width + x] = renderer->back_buffer[new_y * window->width + new_x];
            }
        }
        break;
    case VGA_WINDOW_MODE_CORNER:
    case VGA_WINDOW_MODE_CENTERED:
        for (uint32_t y = 0; y < window->height; ++y) {
            uint32_t off_y = window->y_offset + y;
            for (uint32_t x = 0; x < window->width; ++x) {
                uint32_t off_x = window->x_offset + x;
                vga_instance->fb[off_y * vga_instance->width + off_x] = renderer->back_buffer[y * window->width + x];
            }
        }
        break;
    case VGA_WINDOW_MODE_SCALED_2X:
        for (uint32_t y = 0; y < window->height * 2; ++y) {
            uint32_t scaled_y = y / 2;
            for (uint32_t x = 0; x < window->width * 2; ++x) {
                uint32_t scaled_x = x / 2;
                vga_instance->fb[y * vga_instance->width + x] = renderer->back_buffer[scaled_y * window->width + scaled_x];
            }
        }
        break;
    default:
        break;
    }
    return 0;
}

int
command_clear_renderer(envid_t source, union Vgaipc *ipc) {
    struct Renderer *renderer = ipc->clear_renderer.renderer;
    memset(renderer->back_buffer, 0, renderer->window->width * renderer->window->height * sizeof(uint32_t));
    return 0;
}

typedef int (*vshandler)(envid_t source, union Vgaipc *ipc);

vshandler handlers[] = {
        [VGAREQ_WINDOW_CREATE] = command_window_create,
        [VGAREQ_WINDOW_DESTROY] = command_window_destroy,
        [VGAREQ_RENDERER_CREATE] = command_renderer_create,
        [VGAREQ_RENDERER_DESTROY] = command_renderer_destroy,
        [VGAREQ_TEXTURE_CREATE] = command_texture_create,
        [VGAREQ_TEXTURE_DESTROY] = command_texture_destroy,
        [VGAREQ_TEXTURE_UPDATE] = command_texture_update,
        [VGAREQ_TEXTURE_COPY] = command_texture_copy,
        [VGAREQ_DISPLAY] = command_display,
        [VGAREQ_CLEAR_RENDERER] = command_clear_renderer};
#define NHANDLERS (sizeof(handlers) / sizeof(handlers[0]))

void
run(void) {
    envid_t source;
    uint32_t recv;
    int perm, res;
    cprintf("GS: running...\n");
    while (1) {
        perm = 0;
        size_t size = PAGE_SIZE;
        recv = ipc_recv(&source, vsreq, &size, &perm);
        if (!(perm & PROT_R)) {
            continue; /* Just leave it hanging... */
        }
        if (recv < NHANDLERS && handlers[recv]) {
            res = handlers[recv](source, vsreq);
        } else {
            res = -1;
        }
        ipc_send(source, res, NULL, PAGE_SIZE, perm);
        sys_unmap_region(0, vsreq, PAGE_SIZE);
    }
}

void
umain(int argc, char **argv) {
    cprintf("GS is running\n");
    bochs_vbe_init(640, 400, VBE_DISPI_BPP_32);
    cprintf("GS: VBE has initialized\n");
    sys_resize_uefi_display(640, 400);
    cprintf("GS: Display has reinited\n");
    run();
}
