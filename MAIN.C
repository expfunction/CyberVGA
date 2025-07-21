#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <bios.h>
#include <math.h>
#include <dos.h>
#include <mem.h>
#include <alloc.h>
#include <time.h>

// Engine heders
#include "cybervga.h"
#include "cv_math.h"
#include "cv_rndr.h"
#include "cv_mesh.h"
#include "cv_camer.h"

// Globals
unsigned long last_fps_time = 0;
int frame_count = 0;
int fps = 0;

void print_fps(int fps)
{
	char far* vga_text = (char far*)MK_FP(0xB800, 0);	// Text mode memory start here
	char msg[20]; int i;

	sprintf(msg, "FPS: %d", fps);
	for(i=0; msg[i]; ++i)
	{
		vga_text[i*2] = msg[i];
		vga_text[i*2+1] = 0x1F;
	}
}

// Test cube as mesh
CV_Vec3 cube_verts[8]=
{
	{-1,-1,-1},
	{1,-1,-1},
	{1,1,-1},
	{-1,1,-1},
	{-1,-1,1},
	{1,-1,1},
	{1,1,1},
	{-1,1,1}
};
int cube_tris[12][3]=
{
	{0,1,2},{0,2,3},
	{4,5,6},{4,6,7},
	{0,1,5},{0,5,4},
	{3,2,6},{3,6,7},
	{0,3,7},{0,7,4},
	{1,2,6},{1,6,5}
};

// Test camera
CV_Camera mainCam=
{
	0,0,-3,	// world pos
	0,0,0,	// look at target
	0,1,0	// up direction
};

// MAIN
int main()
{
	Byte far* VGA = (Byte far*)MK_FP(0xA000, 0);
	Byte far* screenBuffer;
	int angleX=30, angleY=30, angleZ=30, i, a, b;
	float angleXf = 30.0f, angleYf = 30.0f, angleZf = 30.0f;
	float roation_speed = 90.0f;							// degs/sec
	float ux,uy,uz, vx,vy,vz, nx, ny, nz, dx1, dx2, dy1, dy2, cross;						// Backface culling
	CV_Vec3 transformed[8];
	CV_Vec2 projected[8];
	CV_Mesh testCube;

	// Timing
	clock_t last_time = clock();
	clock_t last_fps_clock = clock();

	cv_init_trig_tables();

	// Initialize screen buffer
	screenBuffer = (Byte far*)farmalloc(CV_SCREENRES);
	if(!screenBuffer)
	{
		printf("Error! Not enough memory.");
		exit(1);
	}

	cv_set_vga_mode();
	cv_make_default_palette();

	//init test mesh
	cv_mesh_init(&testCube, cube_verts, 8, cube_tris, 12);

	while(!bioskey(1))
	{
		clock_t now = clock();
		float deltaTime = (now - last_time) / (float)CLOCKS_PER_SEC;
		last_time = now;

		// Clear screen buffer
	   _fmemset((Byte far*)screenBuffer, 0, CV_SCREENRES);

	   // Update rotation based on time
	   angleXf += 50.0f * deltaTime;
	   angleYf += 32.0f * deltaTime;
	   //angleZf += 0.0f * deltaTime;
	   angleX = ((int)angleXf) % CV_TABLESIZE;
	   angleY = ((int)angleYf) % CV_TABLESIZE;
	   angleZ = ((int)angleZf) % CV_TABLESIZE;

		// Transform & project
		for(i=0; i < testCube.num_verts; ++i)
		{
			// Backface culling
			float dot;
			const CV_Vec3 t0 = transformed[testCube.tris[i][0]];
			const CV_Vec3 t1 = transformed[testCube.tris[i][1]];
			const CV_Vec3 t2 = transformed[testCube.tris[i][2]];

			CV_Vec3 v0,v1,v2,cV;	// Face to camera direction vectors

			transformed[i] = testCube.verts[i];
			cv_rotate(&transformed[i], angleX, angleY, angleZ);

			ux = t1.x - t0.x; vx = t2.x - t0.x;
			uy = t1.y - t0.y; vy = t2.y - t0.y;
			uz = t1.z - t0.z; vz = t2.z - t0.z;

			nx = uy * vz - uz * vy;
			ny = uz * vx - ux * vz;
			nz = ux * vy - uy * vx;

			v0.x = mainCam.x - t0.x; v0.y = mainCam.y - t0.y;
			v0.z = mainCam.z - t0.z;

			dot = nx*v0.x + ny*v0.y + nz*v0.z;

			if(dot<0) continue;

			projected[i] = cv_project(transformed[i], &mainCam);
		}

		// Draw tris
		for(i=0; i<testCube.num_tris; ++i)
		{
			const CV_Vec2* v0 = &projected[testCube.tris[i][0]];
			const CV_Vec2* v1 = &projected[testCube.tris[i][1]];
			const CV_Vec2* v2 = &projected[testCube.tris[i][2]];
			if(!v0 || !v1 || !v2) continue;

			cv_draw_line(v0->x, v0->y, v1->x, v1->y, 62, screenBuffer);
//			cv_draw_triangle_fill(v0, v1, v2, 22, screenBuffer);
//			cv_draw_triangle_wire(v0, v1, v2, 63, screenBuffer);
		}
		// FPS calculations
		if((now - last_fps_clock)>=CLOCKS_PER_SEC)
		{
			fps = frame_count;
			frame_count = 0;
			last_fps_clock = now;
		}

		// Copy screen buffer to vga memory
		_fmemcpy(VGA, screenBuffer, CV_SCREENRES);
	}

	while(bioskey(1)) bioskey(0);

	cv_set_text_mode();

	farfree(screenBuffer);
	printf("FPS: %d\nPress any key to exit.");
	getch();
	return 0;
}
