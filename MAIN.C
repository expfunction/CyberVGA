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


// Cube vertices
CV_Vec3 cube[8] =
{
	{ -1, -1, -1 }, { 1, -1, -1 }, { 1, 1, -1 }, { -1, 1, -1 },
	{ -1, -1,  1 }, { 1, -1,  1 }, { 1, 1,  1 }, { -1, 1,  1 }
};

// Edges
int edges[12][2] =
{
	{0,1},{1,2},{2,3},{3,0}, {4,5},{5,6},{6,7},{7,4}, {0,4},{1,5},{2,6},{3,7}
};

// MAIN
int main()
{
	Byte far* VGA = (Byte far*)MK_FP(0xA000, 0);
	Byte far* screenBuffer;
	int angleX=30, angleY=30, angleZ=30, i, a, b;
	float angleXf = 30.0f, angleYf = 30.0f, angleZf = 30.0f;
	float roation_speed = 90.0f;	// degs/sec
	CV_Vec3 transformed[8];
	CV_Vec2 projected[8];

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
	   angleZf += 90.0f * deltaTime;
	   angleX = ((int)angleXf) % CV_TABLESIZE;
	   angleY = ((int)angleYf) % CV_TABLESIZE;
	   angleZ = ((int)angleZf) % CV_TABLESIZE;

		// Transform & project
		for(i=0; i<8; i++)
		{
			transformed[i] = cube[i];
			cv_rotate(&transformed[i], angleX, angleY, angleZ);
			projected[i] = cv_project(transformed[i]);
		}
		// Draw edges
		for(i=0; i<12; i++)
		{
			a=edges[i][0]; b=edges[i][1];
			cv_draw_line(projected[a].x, projected[a].y , projected[b].x, projected[b].y, 1, screenBuffer);
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
