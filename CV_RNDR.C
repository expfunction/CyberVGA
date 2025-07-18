#include "cv_rndr.h"

// Draw a pixel
void cv_put_pixel(int x, int y, Byte color, Byte far* buffer)
{
	if(x <= 0 || x >= CV_WIDTH || y <= 0 || y >= CV_HEIGHT) return;
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

void cv_draw_triangle_wire(const CV_Vec2* v0, const CV_Vec2* v1,
						   const CV_Vec2* v2, Byte color, Byte far* buffer)
{
	cv_draw_line(v0->x, v0->y, v1->x, v1->y, color, buffer);
	cv_draw_line(v1->x, v1->y, v2->x, v2->y, color, buffer);
	cv_draw_line(v2->x, v2->y, v0->x, v0->y, color, buffer);
}

void cv_draw_triangles_wire(const CV_Vec2* vertices, const int* indices,
							int numTris, Byte color, Byte far* buffer)
{
	int i;
	for(i=0; i<numTris; ++i)
	{
		const CV_Vec2* v0 = &vertices[indices[i*3]];
		const CV_Vec2* v1 = &vertices[indices[i*3+1]];
		const CV_Vec2* v2 = &vertices[indices[i*3+2]];
		cv_draw_triangle_wire(v0, v1, v2, color, buffer);
	}
}

void cv_draw_triangle_fill(const CV_Vec2* v0, const CV_Vec2* v1,
						   const CV_Vec2* v2, Byte color, Byte far* buffer)
{
	// Vars
	const CV_Vec2* pv0 = v0;
    	const CV_Vec2* pv1 = v1;
    	const CV_Vec2* pv2 = v2;
    	int y0, y1, y2, x0, x1, x2, y, xa, xb, xstart, xend;
    	float slope1, slope2, fx1, fx2;

    	// Sort vertices by Y (pv0->y <= pv1->y <= pv2->y)
    	if(pv1->y < pv0->y) swap_vec2(&pv1, &pv0);
    	if(pv2->y < pv0->y) swap_vec2(&pv2, &pv0);
    	if(pv2->y < pv1->y) swap_vec2(&pv2, &pv1);

   	// Convert to ints for scanlines
    	y0 = (int)(pv0->y + 0.5f);
    y1 = (int)(pv1->y + 0.5f);
    y2 = (int)(pv2->y + 0.5f);
    x0 = (int)(pv0->x + 0.5f);
    x1 = (int)(pv1->x + 0.5f);
    x2 = (int)(pv2->x + 0.5f);

    if(y1 == y0) {
        // Flat-top
        slope1 = (float)(x2 - x0) / (y2 - y0);
        slope2 = (float)(x2 - x1) / (y2 - y1);
        fx1 = (float)x0;
        fx2 = (float)x1;
        for(y = y0; y <= y2; ++y) {
            xa = (int)(fx1 + 0.5f);
            xb = (int)(fx2 + 0.5f);
            if(xa > xb) { int t = xa; xa = xb; xb = t; }
            for(xstart = xa; xstart <= xb; ++xstart)
                cv_put_pixel(xstart, y, color, buffer);
            fx1 += slope1;
            fx2 += slope2;
        }
    } else if(y2 == y1) {
        // Flat-bottom
        slope1 = (float)(x1 - x0) / (y1 - y0);
        slope2 = (float)(x2 - x0) / (y2 - y0);
        fx1 = (float)x0;
        fx2 = (float)x0;
        for(y = y0; y <= y1; ++y) {
            xa = (int)(fx1 + 0.5f);
            xb = (int)(fx2 + 0.5f);
            if(xa > xb) { int t = xa; xa = xb; xb = t; }
            for(xstart = xa; xstart <= xb; ++xstart)
                cv_put_pixel(xstart, y, color, buffer);
            fx1 += slope1;
            fx2 += slope2;
        }
    } else {
        // General triangle: split
        float alpha = (float)(y1 - y0) / (float)(y2 - y0);
        int x3 = x0 + (int)((x2 - x0) * alpha + 0.5f);
        int y3 = y1;

        // Fill lower part
        slope1 = (float)(x1 - x0) / (y1 - y0);
        slope2 = (float)(x3 - x0) / (y3 - y0);
        fx1 = (float)x0;
        fx2 = (float)x0;
        for(y = y0; y <= y1; ++y) {
            xa = (int)(fx1 + 0.5f);
            xb = (int)(fx2 + 0.5f);
            if(xa > xb) { int t = xa; xa = xb; xb = t; }
            for(xstart = xa; xstart <= xb; ++xstart)
                cv_put_pixel(xstart, y, color, buffer);
            fx1 += slope1;
            fx2 += slope2;
        }
        // Fill upper part
        slope1 = (float)(x2 - x1) / (y2 - y1);
        slope2 = (float)(x2 - x3) / (y2 - y3);
        fx1 = (float)x1;
        fx2 = (float)x3;
        for(y = y1; y <= y2; ++y) {
            xa = (int)(fx1 + 0.5f);
            xb = (int)(fx2 + 0.5f);
            if(xa > xb) { int t = xa; xa = xb; xb = t; }
            for(xstart = xa; xstart <= xb; ++xstart)
                cv_put_pixel(xstart, y, color, buffer);
            fx1 += slope1;
            fx2 += slope2;
        }
    }
}