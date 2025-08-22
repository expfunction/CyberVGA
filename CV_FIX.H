#ifndef CV_FIX_H
#define CV_FIX_H

#include <stdlib.h>
typedef long fx;		//32-bit signed fixed point definition

#define FX_SHIFT	16
#define FX_ONE		(1L << FX_SHIFT)
#define TO_FX(i)	((fx)((i) << FX_SHIFT))
#define FROM_FX(i)	((int)((i) >> FX_SHIFT))
#define FX_MUL(a,b) ((fx)(((long long)(a)*(long long)(b)) >> FX_SHIFT))
#define FX_DIV(a,b) ((fx)(((long long)(a) << FX_SHIFT) / (b)))

typedef struct {fx x, y, z;} CV_Vec3fx;

#endif