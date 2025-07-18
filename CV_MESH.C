#include "cv_mesh.h"

void cv_mesh_init(CV_Mesh* mesh, CV_Vec3* verts,
				  int nVerts, const int tris[][3], int nTris)
{
	int i;
	if(nVerts > CV_MESH_MAX_VERTS) nVerts =CV_MESH_MAX_VERTS;
	if(nTris > CV_MESH_MAX_TRIS) nTris =CV_MESH_MAX_TRIS;

	mesh->num_verts = nVerts;
	mesh->num_tris = nTris;


	for(i=0; i<nVerts; ++i)
		mesh->verts[i] = verts[i];
	for(i=0; i<nTris; ++i)
	{
		mesh->tris[i][0] = tris[i][0];
		mesh->tris[i][1] = tris[i][1];
		mesh->tris[i][2] = tris[i][2];
	}
}
