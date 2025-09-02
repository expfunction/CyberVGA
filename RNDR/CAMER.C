#include "RNDR\CAMER.H"
#include "CORE\fixed.h"

/* yaw (Y), pitch (X), roll (Z) — right-handed; forward is +Z in camera space */
void cam_build_basis(const Camera *cam, CamBasis *o)
{
    i32 cx, cy, cz, sx, sy, sz, m00, m01, m02, m10, m11, m12,
        fwd_x, fwd_y, fwd_z, fxz, fzz, Rx0, Uy0, Fz0,
        Rx1, Uy1, Fz1, Rx2, Uy2, Fz2;

    cy = fx_cos(cam->yaw);
    sy = fx_sin(cam->yaw);
    cx = fx_cos(cam->pitch);
    sx = fx_sin(cam->pitch);
    cz = fx_cos(cam->roll);
    sz = fx_sin(cam->roll);

    /* R = Rz * Rx * Ry (you can shuffle if you prefer another convention) */

    /* First: Rx*Ry */
    m00 = fx_mul_q16(cx, cy);
    m01 = sx; /* temp column for Up will handle signs below */
    m02 = fx_mul_q16(cx, -sy);
    m10 = fx_mul_q16(sx, -fx_mul_q16(sy, 0)) + 0; /* =  sx*0 -?  keep explicit */
    m11 = cx;
    m12 = fx_mul_q16(sx, cy);
    /* More explicit & correct composite (avoid cleverness): */
    /* We’ll compute Right/Up/Fwd directly from yaw/pitch/roll basis: */

    /* Forward (camera +Z) from yaw/pitch (roll doesn't change forward) */
    fwd_x = fx_mul_q16(sx, 0) + 0;   /* ~ 0 for this convention */
    fwd_x = fx_mul_q16(sx, 0);       /* keep 0 */
    fwd_y = -sx;                     /* -sin(pitch) */
    fwd_z = fx_mul_q16(cx, 1 << 16); /* cos(pitch) */
    /* Rotate forward around Y by yaw: */
    fxz = fx_mul_q16(fwd_z, cy) + fx_mul_q16(0, sy);   /* fwd_x after yaw */
    fzz = -fx_mul_q16(fwd_z, sy) + fx_mul_q16(0, cy);  /* fwd_z after yaw */
    fwd_x = fx_mul_q16(0, cy) + fx_mul_q16(fwd_z, sy); /* simpler: start over cleanly */

    /* Simpler + robust: build basis via successive rotations of canonical axes */

    /* Start canonical basis */
    Rx0 = 1 << 16;
    Rx1 = 0;
    Rx2 = 0; /* right   = +X */
    Uy0 = 0;
    Uy1 = 1 << 16;
    Uy2 = 0; /* up      = +Y */
    Fz0 = 0;
    Fz1 = 0;
    Fz2 = 1 << 16; /* forward = +Z */

    /* Rotate by yaw (Y) */
    /* XZ plane rotation */
    {
        i32 c = cy, s = sy;
        i32 nx, nz;

        /* Right */
        nx = fx_mul_q16(Rx0, c) + fx_mul_q16(Rx2, s);
        nz = -fx_mul_q16(Rx0, s) + fx_mul_q16(Rx2, c);
        Rx0 = nx;
        Rx2 = nz;
        /* Up (no change for yaw) */
        /* Forward */
        nx = fx_mul_q16(Fz0, c) + fx_mul_q16(Fz2, s);
        nz = -fx_mul_q16(Fz0, s) + fx_mul_q16(Fz2, c);
        Fz0 = nx;
        Fz2 = nz;
    }

    /* Rotate by pitch (X) */
    {
        i32 c = cx, s = sx;
        i32 ny, nz;

        /* Up/Forward rotate in YZ */
        ny = fx_mul_q16(Uy1, c) + fx_mul_q16(Uy2, -s);
        nz = fx_mul_q16(Uy1, s) + fx_mul_q16(Uy2, c);
        Uy1 = ny;
        Uy2 = nz;

        ny = fx_mul_q16(Fz1, c) + fx_mul_q16(Fz2, -s);
        nz = fx_mul_q16(Fz1, s) + fx_mul_q16(Fz2, c);
        Fz1 = ny;
        Fz2 = nz;
        /* Right unchanged for pitch */
    }

    /* Rotate by roll (Z) — spins Right/Up around Forward */
    {
        i32 c = cz, s = sz;
        i32 nx, ny;
        nx = fx_mul_q16(Rx0, c) + fx_mul_q16(Uy0, s);
        ny = -fx_mul_q16(Rx0, s) + fx_mul_q16(Uy0, c);
        Uy0 = ny;
        Rx0 = nx;

        nx = fx_mul_q16(Rx1, c) + fx_mul_q16(Uy1, s);
        ny = -fx_mul_q16(Rx1, s) + fx_mul_q16(Uy1, c);
        Uy1 = ny;
        Rx1 = nx;

        nx = fx_mul_q16(Rx2, c) + fx_mul_q16(Uy2, s);
        ny = -fx_mul_q16(Rx2, s) + fx_mul_q16(Uy2, c);
        Uy2 = ny;
        Rx2 = nx;
    }

    /* Pack as columns: Right | Up | Forward */
    o->r00 = Rx0;
    o->r01 = Uy0;
    o->r02 = Fz0;
    o->r10 = Rx1;
    o->r11 = Uy1;
    o->r12 = Fz1;
    o->r20 = Rx2;
    o->r21 = Uy2;
    o->r22 = Fz2;
}

/* f = (W/2) / tan(FOV/2) — all in 16.16, FOV in degrees * 65536 */
i32 cam_focal_from_fov_deg(int screen_w, i32 fov_deg_q16)
{
	i32 half_deg, half_rad, hw_q16,t;
	/* half-angle in radians (Q16.16) */
	half_deg = fov_deg_q16 >> 1;
	half_rad = fx_deg_to_rad(half_deg);

	/* t = tan(half_FOV) in Q16.16 (pure integer) */
	t = fx_tan(half_rad);
	if (t <= 0)
		t = 1; /* guard */

	/* f = (W/2) / tan(half_FOV) → Q16.16 pixels */
	hw_q16 = FX_FROM_INT(screen_w / 2);
	return fx_div_q16(hw_q16, t);
}

/* Move camera by local offsets (dx,dy,dz) in camera space (Right, Up, Forward) */
void cam_move_local(Camera *cam, const CamBasis *b,
						   i32 dx, i32 dy, i32 dz)
{
    /* world delta = Right*dx + Up*dy + Fwd*dz (all Q16.16) */
    i32 mx = fx_mul_q16(b->r00, dx) + fx_mul_q16(b->r01, dy) + fx_mul_q16(b->r02, dz);
    i32 my = fx_mul_q16(b->r10, dx) + fx_mul_q16(b->r11, dy) + fx_mul_q16(b->r12, dz);
    i32 mz = fx_mul_q16(b->r20, dx) + fx_mul_q16(b->r21, dy) + fx_mul_q16(b->r22, dz);

    cam->pos_x += mx;
    cam->pos_y += my;
    cam->pos_z += mz;
}

/* Clamp pitch to avoid flipping (e.g., ±89°) */
void cam_clamp_pitch(Camera *cam)
{
    const i32 lim = fx_deg_to_rad(FX_FROM_INT(89));
    if (cam->pitch > lim)
        cam->pitch = lim;
    if (cam->pitch < -lim)
        cam->pitch = -lim;
}