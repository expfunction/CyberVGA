#include <bios.h>
#include "IO\IO.H"

// Init io
void cv_io_init(void)
{
}

// Return non zero if key pressed
sint cv_io_key_pressed(void)
{
	return bioskey(1) != 0;
}

// Returns pressed key as scancode
u8 cv_io_poll(void)
{
	unsigned int key;
	if (!cv_io_key_pressed())
		return 0;
	key = bioskey(0);
	return (key >> 8) & 0xFF;
}
