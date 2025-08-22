#ifndef CV_MATH_H
#define CV_MATH_H

#include "cv_fix.h"
#include "cv_camer.h"

#define CV_PI 3.14159265
#define CV_TABLESIZE 360

// 3D/2D vertex
typedef struct { float x, y, z; } CV_Vec3;
typedef struct { int x, y; } CV_Vec2;

// Vertex operations
static void swap_vec2(const CV_Vec2** a, const CV_Vec2** b)
{
	const CV_Vec2* t = *a; *a = *b; *b=t;
}

void cv_init_trig_tables(void);
void cv_init_trig_tables_fx(void);

void cv_rotate_fx(CV_Vec3fx* v, int ax, int ay, int az);
void cv_rotate(CV_Vec3* v, int ax, int ay, int az);

void cv_project_fx(const CV_Vec3fx v, const CV_Camera* cam, int* sx, int* sy);
CV_Vec2 cv_project(const CV_Vec3 v, const CV_Camera* cam);

#endif