#include <math.h>
#include "cv_math.h"
#include "cybervga.h"

static fx sinTableFx[CV_TABLESIZE];
static fx cosTableFx[CV_TABLESIZE];

static float sinTable[CV_TABLESIZE];
static float cosTable[CV_TABLESIZE];

void cv_init_trig_tables_fx(void)
{
	int i;
	for(i=0; i<CV_TABLESIZE; ++i)
	{
		double rad = (double)i * (double)CV_PI / 180.0f;
		sinTableFx[i] = (fx)sin(rad * (double)FX_ONE);
		cosTableFx[i] = (fx)cos(rad * (double)FX_ONE);
	}
}

void cv_init_trig_tables(void)
{
	int i;
	for(i=0; i<CV_TABLESIZE; ++i)
	{
		float rad = i * CV_PI / 180.0f;
		sinTable[i] = sin(rad);
		cosTable[i] = cos(rad);
	}
}

void cv_rotate_fx(CV_Vec3fx* v, int ax, int ay, int az)
{
	fx x, y, z, sx, sy, sz, cx, cy, cz, x1, y1, z1, x2, y2, z2;

	x = v->x; y = v->y; z = v->z;

	sx = sinTableFx[ax]; cx = cosTableFx[ax];
	sy = sinTableFx[ay]; cy = cosTableFx[ay];
	sz = sinTableFx[az]; cz = cosTableFx[az];

	y1 = FX_MUL(y, cx) - FX_MUL(z, sx);
	z1 = FX_MUL(y, sx) + FX_MUL(z, cx);
	y=y1; z=z1;

	x1 = FX_MUL(x, cy) + FX_MUL(z, sy);
	z2 = -FX_MUL(x, sy) + FX_MUL(z, cy);
	x=x1; z=z2;

	x2 = FX_MUL(x, cz) - FX_MUL(y, sz);
	y2 = FX_MUL(x, sz) + FX_MUL(y, cz);

	v->x=x2; v->y = y2; v->z = z2;
}

// 3D rotation using float
void cv_rotate(CV_Vec3* v, int ax, int ay, int az)
{
	float x = v->x, y = v->y, z = v->z;
	float sx = sinTable[ax], cx = cosTable[ax];
	float sy = sinTable[ay], cy = cosTable[ay];
	float sz = sinTable[az], cz = cosTable[az];
	float x1, x2, y1, y2, z1, z2;

	// Rotate X
	y1 = y * cx - z * sx;
	z1 = y * sx + z * cx;
	y = y1; z = z1;
	// Rotate Y
	x1 = x * cy + z * sy;
	z2 = -x * sy + z * cy;
	x = x1; z = z2;
	// Rotate Z
	x2 = x * cz - y * sz;
	y2 = x * sz + y * cz;

	v->x = x2; v->y = y2; v->z = z2;
}

// Perspective projection in fixed point
void cv_project_fx(const CV_Vec3fx v, const CV_Camera* cam, int* sx, int* sy)
{
	fx camx, camy, camz, distance, scale, vx, vy, vz, denom, xs, ys;

	camx = (fx)(cam->x * FX_ONE);
	camy = (fx)(cam->y * FX_ONE);
	camz = (fx)(cam->z * FX_ONE);

	distance = TO_FX(3);
	scale = TO_FX(90);

	vx = v.x - camx;
	vy = v.y - camy;
	vz = v.z - camz;

	denom = vz + distance;
	if(denom < (1<<8)) denom = (1<<8);	// To avoid /0

	xs = FX_DIV(FX_MUL(vx, scale), denom);
	ys = FX_DIV(FX_MUL(vy, scale), denom);

	*sx = FROM_FX(xs) + (CV_WIDTH >> 1);
	*sy = (CV_HEIGHT >> 1) - FROM_FX(ys);
}

// Perspective projection in float
CV_Vec2 cv_project(const CV_Vec3 v, const CV_Camera* cam)
{
	float distance = 3.0f;
	float scale = 90.0f;
	float vx = v.x - cam->x;
	float vy = v.y - cam->y;
	float vz = v.z - cam->z;

	float denom = vz + distance;

	CV_Vec2 result;

	if(denom < 0.1f) denom = 0.1f; // prevent div by zero
	result.x = (int)((vx * scale) / denom + CV_WIDTH / 2);
	result.y = (int)(CV_HEIGHT/2 - ((vy * scale)/denom));
	return result;
}
