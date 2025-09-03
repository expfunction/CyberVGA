#ifndef WIRE_H
#define WIRE_H
#include "CORE\TYPES.H"
#include "MESH\CVG1.H"

typedef struct
{
    sint w, h;
    u8 *back;
} Surface8;

void wire_clear(Surface8 *s, u8 c);
void wire_line(Surface8 *s, sint x0, sint y0, sint x1, sint y1, u8 c);
void wire_project16_16(i32 x, i32 y, i32 z, i32 f_fx, sint *sx, sint *sy);
void wire_rotate_yx(i32 x, i32 y, i32 z, i32 siny, i32 cosy, i32 sinx, i32 cosx,
                    i32 *ox, i32 *oy, i32 *oz);
void wire_draw_mesh_edges(const MeshCVG1 *m, const sint *sx, const sint *sy, Surface8 *s, u8 c);

/* New: culled wire draw */
void wire_draw_mesh_edges_culled(const MeshCVG1 *m, const sint *sx, const sint *sy,
                                 Surface8 *s, u8 color, sint ccw_front);

#endif
