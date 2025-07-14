#include "cybervga.h"

// Set screen to VGA mode 13h
void cv_set_vga_mode(void)
{
	union REGS regs;
	regs.h.ah = 0x00;
	regs.h.al = 0x13;
	int86(0x10, &regs, &regs);
}
// Set screen back to text mode
void cv_set_text_mode(void)
{
	union REGS regs;
	regs.h.ah = 0x00;
	regs.h.al = 0x03;
	int86(0x10, &regs, &regs);
}
