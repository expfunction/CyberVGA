#include <string.h>
#include <dos.h>
#include "GFX\VGA.H"

void back_draw_border(u8 *back)
{
    int x, y;
    for (y = 0; y < SCREEN_H; y++)
        for (x = 0; x < SCREEN_W; x++)
            back[y * SCREEN_W + x] = 0; /* black */
}

void vga_set_mode13h(void)
{
    union REGS r;
    r.h.ah = 0x00;
    r.h.al = 0x13;
    int386(0x10, &r, &r);
}

void vga_reset_text(void)
{
    union REGS r;
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int386(0x10, &r, &r);
}
void vga_blit(const u8 *buf)
{
    /* ALWAYS use your ASM path; no DPMI 0x0800 mapping, no gating */
    vga_blit_8(buf);
}
