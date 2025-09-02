#ifndef WIRE_H
#define WIRE_H
#include "MESH\CVG1.H"

typedef struct
{
    int w, h;
    unsigned char *back;
} Surface8;

void wire_clear(Surface8 *s, unsigned char c);
void wire_line(Surface8 *s, int x0, int y0, int x1, int y1, unsigned char c);
void wire_project16_16(i32 x, i32 y, i32 z, i32 f_fx, int *sx, int *sy);
void wire_rotate_yx(i32 x, i32 y, i32 z, i32 siny, i32 cosy, i32 sinx, i32 cosx,
                    i32 *ox, i32 *oy, i32 *oz);
void wire_draw_mesh_edges(const MeshCVG1 *m, const int *sx, const int *sy, Surface8 *s, unsigned char c);

/* New: culled wire draw */
void wire_draw_mesh_edges_culled(const MeshCVG1 *m, const int *sx, const int *sy,
                                 Surface8 *s, unsigned char color, int ccw_front);

#endif
