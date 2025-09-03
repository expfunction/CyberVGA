#ifndef CAMER_H
#define CAMER_H

#include "MESH\CVG1.H"  /* i32,u32 */
#include "CORE\FIXED.H" /* fx_sin/fx_cos, fx_mul_q16/fx_div_q16, FX_FROM_INT */

typedef struct
{
    /* Pose (world space) */
    i32 pos_x, pos_y, pos_z; /* 16.16 */
    i32 yaw, pitch, roll;    /* 16.16 radians */

    /* Lens */
    i32 f_fx;  /* focal length in pixels (16.16) */
    i32 znear; /* near plane distance (16.16), e.g. FX_FROM_INT(1) */
} Camera;

/* Derived basis (view rotation) — 16.16 columns */
typedef struct
{
    i32 r00, r01, r02; /* right.x,  up.x,  fwd.x */
    i32 r10, r11, r12; /* right.y,  up.y,  fwd.y */
    i32 r20, r21, r22; /* right.z,  up.z,  fwd.z */
} CamBasis;

/* Build camera basis from yaw/pitch/roll (Z = forward), 16.16 */
void cam_build_basis(const Camera *cam, CamBasis *out);

/* World  View: v_cam = R^T * (v_world - cam.pos) */
static void cam_world_to_view(const CamBasis *b,
                              i32 wx, i32 wy, i32 wz,
                              i32 cx, i32 cy, i32 cz,
                              i32 *outx, i32 *outy, i32 *outz)
{
    /* d = w - cam.pos */
    i32 dx = wx - cx;
    i32 dy = wy - cy;
    i32 dz = wz - cz;

    /* R^T * d  (columns are right, up, fwd) */
    *outx = fx_mul_q16(dx, b->r00) + fx_mul_q16(dy, b->r10) + fx_mul_q16(dz, b->r20); /* view X */
    *outy = fx_mul_q16(dx, b->r01) + fx_mul_q16(dy, b->r11) + fx_mul_q16(dz, b->r21); /* view Y */
    *outz = fx_mul_q16(dx, b->r02) + fx_mul_q16(dy, b->r12) + fx_mul_q16(dz, b->r22); /* view Z (forward) */
}

/* Compute focal length from FOV (degrees) and screen width.
   f = (W/2) / tan(FOV/2). Returns 16.16 pixels. */
i32 cam_focal_from_fov_deg(int screen_w, i32 fov_deg_q16);

/* Move camera by local offsets (dx,dy,dz) in camera space (Right, Up, Forward) */
void cam_move_local(Camera *cam, const CamBasis *b,
                    i32 dx, i32 dy, i32 dz);
/* Clamp pitch to avoid flipping (e.g., ±89°) */
void cam_clamp_pitch(Camera *cam);
#endif
