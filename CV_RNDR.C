#include "cv_rndr.h"

// Draw a pixel
void cv_put_pixel(int x, int y, Byte color, Byte far* buffer)
{
	if(x < 0 || x >= CV_WIDTH || y < 0 || y >= CV_HEIGHT) return;
	buffer[y * CV_WIDTH + x] = color;
}

// Bresenham's line
void cv_draw_line(int x0, int y0, int x1, int y1, Byte color, Byte far* buffer)
{
	int dx, sx, dy, sy, err, e2;

	dx = abs(x1 - x0);
	sx = x0 < x1 ? 1 : -1;

	dy = -abs(y1 - y0);
	sy = y0 < y1 ? 1 : -1;
	err = dx + dy;

	while(1)
	{
		cv_put_pixel(x0, y0, color, buffer);
		if(x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if(e2 >= dy) { err += dy; x0 += sx; }
		if(e2 <= dx) { err += dx; y0 += sy; }
	}
}
