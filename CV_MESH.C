#include <stdio.h>
#include <string.h>
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

int cv_mesh_load(CV_Mesh* mesh, const char* filename)
{
	int vcount=0, fcount=0;
	char line[256];
	FILE* file = fopen(filename, "r");

	if(!file)
	{
		printf("File error: %s", filename);
		return 0;
	}

	while(fgets(line, sizeof(line), file))
	{
		if(line[0]=='v'&&line[1]==' ')
		{
			if(vcount >= CV_MESH_MAX_VERTS) continue;
			sscanf(line, "v %f %f %f",&mesh->verts[vcount].x,&mesh->verts[vcount].y,&mesh->verts[vcount].z);
			vcount++;
		}
		else if(line[0]=='f' && line[1]==' ')
		{
			char* token;
			int indices[3];
			int i0,i1,i2,j=0;

			if(fcount >= CV_MESH_MAX_TRIS) continue;

			token = strtok(line+2, " \t\n\r");
			while(token && j<3)
			{
				char vbuf[16];
				int vi=-1,k = 0;
				while(*token && *token != '/' && k < 15)
				{
					vbuf[k++] = *token;
					token++;
				}
				vbuf[k]='\0';

				vi = atoi(vbuf);
				if(vi<=0) break;

				indices[j++] = vi-1;
				token = strtok(NULL, " \t\n\r");
			}
			if(j==3)
			{
				mesh->tris[fcount][0] = indices[0];
				mesh->tris[fcount][1] = indices[1];
				mesh->tris[fcount][2] = indices[2];
				fcount++;
			}
		}
	}
	fclose(file);
	mesh->num_verts = vcount;
	mesh->num_tris = fcount;
	return 1;
}