#include <inc/lib.h>
#include <inc/vga.h>


union Vgaipc vgaipcbuf __attribute__((aligned(PAGE_SIZE)));

static int
__vga_ipc_process(vga_command command_type, void *dstva) {
    static envid_t vga_serv_env;
    if (!vga_serv_env) vga_serv_env = ipc_find_env(ENV_TYPE_VGA);
    if (debug) cprintf("ipc_find_env(ENV_TYPE_VGA) = %d\n", vga_serv_env);

    ipc_send(vga_serv_env, command_type, &vgaipcbuf, PAGE_SIZE, PROT_RW);
    if (debug) cprintf("Send complete\n");
    size_t maxsz = PAGE_SIZE;
    return ipc_recv_from(NULL, dstva, &maxsz, NULL, vga_serv_env);
}

Window_ptr
vga_window_create(uint32_t width, uint32_t height, vga_window_mode mode) {
    vgaipcbuf.window_create.width = width;
    vgaipcbuf.window_create.height = height;
    vgaipcbuf.window_create.mode = mode;
    if (debug) cprintf("send window_create\n");
    int res;
    if ((res = __vga_ipc_process(VGAREQ_WINDOW_CREATE, &vgaipcbuf))) {
        return NULL;
    }
    if (debug) cprintf("send window_create completed %d\n", res);
    return vgaipcbuf.window_create_ret.window;
}

int
vga_window_destroy(Window_ptr window) {
    vgaipcbuf.destroy_window.window = window;
    return __vga_ipc_process(VGAREQ_WINDOW_DESTROY, &vgaipcbuf);
}

Texture_ptr
vga_texture_create(uint32_t width, uint32_t height, bool need_mapping, uint32_t **buffer_map) {
    vgaipcbuf.texture_create.width = width;
    vgaipcbuf.texture_create.height = height;
    vgaipcbuf.texture_create.need_mapping = need_mapping;

    int res = __vga_ipc_process(VGAREQ_TEXTURE_CREATE, &vgaipcbuf);
    if (res)
        return NULL;

    if (need_mapping) {
        *buffer_map = vgaipcbuf.texture_create_ret.buffer_map;
#ifdef SANITIZE_USER_SHADOW_BASE
        platform_asan_unpoison(*buffer_map, sizeof(uint32_t) * width * height);
#endif
    }

    return vgaipcbuf.texture_create_ret.texture;
}

int
vga_texture_destroy(Texture_ptr texture) {
    vgaipcbuf.texture_destroy.texture = texture;
#ifdef SANITIZE_USER_SHADOW_BASE
    platform_asan_poison(texture->mapped_user_buf, sizeof(uint32_t) * texture->width * texture->height);
#endif
    return __vga_ipc_process(VGAREQ_TEXTURE_DESTROY, &vgaipcbuf);
}

Renderer_ptr
vga_renderer_create(Window_ptr window) {
    vgaipcbuf.renderer_create.window = window;
    int res = __vga_ipc_process(VGAREQ_RENDERER_CREATE, &vgaipcbuf);
    if (res)
        return NULL;

    return vgaipcbuf.renderer_create_ret.renderer;
}

int
vga_renderer_destroy(Renderer_ptr renderer) {
    vgaipcbuf.renderer_destroy.renderer = renderer;
    return __vga_ipc_process(VGAREQ_RENDERER_DESTROY, &vgaipcbuf);
}

int
vga_texture_update(Texture_ptr texture, char *buffer, size_t size) {
    vgaipcbuf.texture_update.texture = texture;
    for (size_t offset = 0; offset < size; offset += sizeof(vgaipcbuf.texture_update.buf)) {
        size_t block_size = MIN(size - offset, sizeof(vgaipcbuf.texture_update.buf));
        memcpy(vgaipcbuf.texture_update.buf, buffer + offset, block_size);
        vgaipcbuf.texture_update.offset = offset;
        vgaipcbuf.texture_update.size = block_size;
        int res = __vga_ipc_process(VGAREQ_TEXTURE_UPDATE, &vgaipcbuf);
        if (res)
            return -1;
    }

    return 0;
}

int
vga_texture_copy(Renderer_ptr renderer, Texture_ptr texture) {
    vgaipcbuf.texture_copy.texture = texture;
    vgaipcbuf.texture_copy.renderer = renderer;
    return __vga_ipc_process(VGAREQ_TEXTURE_COPY, &vgaipcbuf);
}

int
vga_display(Renderer_ptr renderer) {
    vgaipcbuf.display.renderer = renderer;
    return __vga_ipc_process(VGAREQ_DISPLAY, &vgaipcbuf);
}

int
vga_clear(Renderer_ptr renderer) {
    vgaipcbuf.clear_renderer.renderer = renderer;
    return __vga_ipc_process(VGAREQ_CLEAR_RENDERER, &vgaipcbuf);
}
