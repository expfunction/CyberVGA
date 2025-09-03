#ifndef VGA_H
#define VGA_H
#include "CORE/TYPES.h"

#define SCREEN_W 320UL
#define SCREEN_H 200UL
#define SCREEN_SIZE (SCREEN_H * SCREEN_W)

void vga_set_mode13h(void);
void vga_reset_text(void);
void vga_blit_8(const u8 *buf); /* always uses ASM blitter */
void vga_set_palette256(const u8 *dac3x256);
void vsync_wait(void);

void vga_blit(const u8 *buf);
void back_draw_border(u8 *back);

#endif
