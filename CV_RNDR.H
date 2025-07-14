#ifndef CV_RNDR_H
#define CV_RNDR_H

#include "cybervga.h"
#include "cv_math.h"

void cv_put_pixel(int x, int y, Byte color, Byte far* buffer);
void cv_draw_line(int x0, int y0, int x1, int y1, Byte color, Byte far* buffer);

#endif