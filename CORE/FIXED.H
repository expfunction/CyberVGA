#ifndef FIXED_H
#define FIXED_H
#include "CORE\TYPES.H"

/* Q16.16 basics */
#define FX_FROM_INT(x) ((i32)((x) << 16))

/* 64-bit-accurate Q16.16 ops (ASM or your existing helpers) */
i32 fx_mul_q16(i32 a, i32 b);     /* (a*b)>>16  */
i32 fx_div_q16(i32 num, i32 den); /* ((num<<16)/den) */

/* ---- Trig: pure fixed-point ---- */
void fixed_init_trig(void); /* builds LUT with CORDIC (no doubles) */
i32 fx_sin(i32 angle16_16); /* angle in radians Q16.16 */
i32 fx_cos(i32 angle16_16); /* angle in radians Q16.16 */

/* Degrees ↔ radians (Q16.16) */
i32 fx_deg_to_rad(i32 deg_q16); /* = deg * (pi/180) */

/* Tangent (Q16.16) */
i32 fx_tan(i32 angle_rad_q16); /* tan = sin/cos via CORDIC */

#endif
