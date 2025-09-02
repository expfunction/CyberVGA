#ifndef VGA_H
#define VGA_H
#include "mesh/cvg1.h"

void vga_set_mode13h(void);
void vga_reset_text(void);
void vga_blit_8(const unsigned char *buf); /* always uses ASM blitter */
void vga_set_palette256(const unsigned char *dac3x256);
void vsync_wait(void);

/* tiny helper while debugging blit */
void back_draw_border(unsigned char *back)
{
    int x, y;
    for (x = 0; x < 320; ++x)
    {
        back[x] = 15;
        back[199 * 320 + x] = 15;
    }
    for (y = 0; y < 200; ++y)
    {
        back[y * 320] = 15;
        back[y * 320 + 319] = 15;
    }
}
#endif
