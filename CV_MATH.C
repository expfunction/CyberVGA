#include <math.h>
#include "cv_math.h"
#include "cybervga.h"

static float sinTable[CV_TABLESIZE];
static float cosTable[CV_TABLESIZE];

void cv_init_trig_tables(void)
{
	int i;
	for(i=0; i<CV_TABLESIZE; ++i)
	{
		sinTable[i] = sin(i * CV_PI / 180.0f);
		cosTable[i] = cos(i * CV_PI / 180.0f);
	}
}

// Perspective projection in float
CV_Vec2 cv_project(const CV_Vec3 v)
{
	float distance = 3.0f;
	float scale = 120.0f;
	float denom = v.z + distance;
	CV_Vec2 result;
	if(denom < 0.1f) denom = 0.1f; // prevent div by zero
	result.x = (int)((v.x * scale) / denom + CV_WIDTH / 2);
	result.y = (int)((v.y * scale) / denom + CV_HEIGHT / 2);
	return result;
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


