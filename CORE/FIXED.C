#include "CORE\fixed.h"

/* ---- Constants (Q16.16) ---- */
#define PI_Q16 205887      /* round(pi * 65536) */
#define DEG2RAD_Q16 1144   /* round(pi/180 * 65536) */
#define CORDIC_K_Q16 39797 /* round(0.607252935 * 65536) */

#define LUT_SIZE 1024
static i32 sinLUT[LUT_SIZE];

/* arctan(2^-i) in Q16.16 for i = 0..15 (CORDIC, rotation mode) */
static const i32 atan_tab_q16[16] = {
    51472, 30386, 16055, 8150, 4091, 2047, 1024, 512,
    256, 128, 64, 32, 16, 8, 4, 2};

/* Convert degrees->radians, all Q16.16 */
i32 fx_deg_to_rad(i32 deg_q16) { return fx_mul_q16(deg_q16, DEG2RAD_Q16); }

/* CORDIC rotation mode: compute cos/sin(angle) in Q16.16 */
static void cordic_sincos_q16(i32 angle_rad_q16, i32 *out_sin, i32 *out_cos)
{
	int i;
	i32 x,y,z;
	/* Map angle into [-pi, pi] to help convergence */
	while (angle_rad_q16 > PI_Q16)
		angle_rad_q16 -= (i32)(2 * PI_Q16);
	while (angle_rad_q16 < -PI_Q16)
		angle_rad_q16 += (i32)(2 * PI_Q16);

	/* Start on the unit vector scaled by K (so we end at 1.0) */
	x = CORDIC_K_Q16;
	y = 0;
	z = angle_rad_q16;

	/* 16 iterations is enough for Q16.16 */
	for (i = 0; i < 16; ++i)
	{
		i32 x_shift, y_shift, z_target;
		x_shift = x >> i;
		y_shift = y >> i;
		z_target = atan_tab_q16[i];

		if (z >= 0)
		{
			x = x - y_shift;
			y = y + x_shift;
            z = z - z_target;
        }
        else
        {
            x = x + y_shift;
            y = y - x_shift;
            z = z + z_target;
        }
    }
    /* x cos, y sin (already Q16.16) */
    if (out_sin)
        *out_sin = y;
    if (out_cos)
        *out_cos = x;
}

/* Build LUT using CORDIC (no floating point anywhere) */
void fixed_init_trig(void)
{
	int i;
    i32 t_q16, angle, s, c;
	/* LUT is indexed by turns (0..2π). We’ll precompute sin for 1024 steps around the circle. */
	for (i = 0; i < LUT_SIZE; ++i)
    {
        /* angle_rad = (i / LUT_SIZE) * 2π  ->  (i * 2π / LUT_SIZE) in Q16.16 */
        /* Compute as: angle = ((i << 16) / LUT_SIZE) * (2π) using Q16 */
		t_q16 = (i32)(((i << 16) / LUT_SIZE));        /* 0..(1.0 - 1/LUT) */
		angle = fx_mul_q16(t_q16, (i32)(2 * PI_Q16)); /* radians Q16 */
        cordic_sincos_q16(angle, &s, &c);
        sinLUT[i] = s;
    }
}

/* Get sin/cos from LUT (input radians). We map radians to a 10-bit turn index. */
static unsigned lut_index_from_rad(i32 angle_rad_q16)
{
	i32 a, t;
	unsigned idx;
	/* Normalize to [0, 2π) then scale to 0..1024 */
	a = angle_rad_q16 % (i32)(2 * PI_Q16);
	if (a < 0)
		a += (i32)(2 * PI_Q16);
	/* index = round( a / (2) * 1024 ) */
	/* a / (2) in Q16: a / (2) == fx_div_q16(a, 2) */
	t = fx_div_q16(a, (i32)(2 * PI_Q16)); /* 0..1 Q16 */
	idx = (unsigned)((t * LUT_SIZE) >> 16) & (LUT_SIZE - 1);
    return idx;
}

i32 fx_sin(i32 angle_rad_q16) { return sinLUT[lut_index_from_rad(angle_rad_q16)]; }
i32 fx_cos(i32 angle_rad_q16)
{
    /* cos(x) = sin(x + π/2) */
    i32 a = angle_rad_q16 + (PI_Q16 >> 1);
    return sinLUT[lut_index_from_rad(a)];
}

/* tan via sin/cos with the same CORDIC core (protect from cos≈0) */
i32 fx_tan(i32 angle_rad_q16)
{
    i32 s, c;
    cordic_sincos_q16(angle_rad_q16, &s, &c);
    /* clamp tiny cos to avoid overflow */
    if (c > -64 && c < 64)
        c = (c < 0) ? -64 : 64;
    return fx_div_q16(s, c);
}
