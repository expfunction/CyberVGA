#ifndef CV_MESH_H
#define CV_MESH_H

#include "cv_math.h"

#define CV_MESH_MAX_VERTS	128UL
#define CV_MESH_MAX_TRIS    256UL

typedef struct
{
	int num_verts, num_tris;
	CV_Vec3 verts[CV_MESH_MAX_VERTS];
	int tris[CV_MESH_MAX_TRIS][3]; 		// Each row {v0,v1,v2}
}CV_Mesh;

void cv_mesh_init(CV_Mesh* mesh, CV_Vec3* verts, int nVerts, const int tris[][3], int nTris);
int cv_mesh_load(CV_Mesh* mesh, const char* filename);

#endif