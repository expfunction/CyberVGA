#ifndef CV_MATH_H
#define CV_MATH_H

#define CV_PI 3.14159265
#define CV_TABLESIZE 360

// 3D/2D vertex
typedef struct { float x, y, z; } CV_Vec3;
typedef struct { int x, y; } CV_Vec2;

void cv_init_trig_tables(void);
void cv_rotate(CV_Vec3* v, int ax, int ay, int az);
CV_Vec2 cv_project(const CV_Vec3 v);

#endif