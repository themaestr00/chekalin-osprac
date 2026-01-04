#include <inc/lib.h>

void 
umain(int argc, char **argv) {
    vsys_sleep(1);
    sys_display_change_vga_state(true);
    cprintf("Create window\n");
    Window_ptr window = vga_window_create(640, 400, VGA_WINDOW_MODE_CENTERED);
    cprintf("Window created: %p\n", window);
    cprintf("Create renderer\n");
    Renderer_ptr renderer = vga_renderer_create(window);
    cprintf("Renderer created: %p\n", renderer);
    cprintf("Create display\n");
    vga_display(renderer);
    cprintf("Display created\n");
    cprintf("Create malloc\n");
    uint32_t *buf = NULL;
    cprintf("Create texture\n");
    Texture_ptr texture = vga_texture_create(640, 400, 1, &buf);
    cprintf("Texture created: %p, buf: %p\n", texture, buf);
    for (int y = 0; y < 400; ++y) {
        for (int x = 0; x < 640; ++x) {
            if (y < 133) buf[y * 640 + x] = 0x00FFFFFF;
            else if (y < 266) buf[y * 640 + x] = 0x000000FF;
            else buf[y * 640 + x] = 0x00FF0000;
        }
    }
    /*for (int i = 200; i < 440 * 400; ++i) {
        buf[i] = 0xFF00FF00;
    }*/
    vsys_sleep(1);
    cprintf("Clear\n");
    vga_clear(renderer);
    vsys_sleep(1);
    cprintf("Texture copy\n");
    vga_texture_copy(renderer, texture);
    vsys_sleep(1);
    cprintf("display\n");
    vga_display(renderer);
    vsys_sleep(1);
    cprintf("forever whatever\n");
    
    sys_display_change_vga_state(false);
    while (1)
        ;
    
}
