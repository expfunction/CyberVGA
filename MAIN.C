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
#include "cv_fix.h"
#include "cv_math.h"
#include "cv_rndr.h"
#include "cv_mesh.h"
#include "cv_camer.h"
#include "cv_io.h"

// Globals
unsigned long last_fps_time = 0;
int frame_count = 0;
int fps = 0;
int running=0;

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

// Test camera
CV_Camera mainCam=
{
	0,0,-2.0f,	// world pos
	0,0,0,		// look at target
	0,1,0		// up direction vector
};

// MAIN
int main()
{
	Byte far* VGA = (Byte far*)MK_FP(0xA000, 0);
	Byte far* screenBuffer;
	int angleX=0, angleY=0, angleZ=0, a, b;
	unsigned long i=0;
	float angleXf = 30.0f, angleYf = 30.0f, angleZf = 30.0f;
	float roation_speed = 90.0f;							// degs/sec
	CV_Mesh testMesh;
	CV_Vec3 far* transformed;
	CV_Vec3fx far* transformedFx;
	CV_Vec2 far* projected;
	int sel=0;
	char info[64];

	// Timing
	clock_t last_time = clock();
	clock_t last_fps_clock = clock();

	// Intro, select mesh
	printf("\nThis is MS-DOS VGA port of Cyber Engine\nPlease Select Mesh File:\n(1)Cube\n(2)Sphere\n");
	scanf("%d",&sel);if(!(sel==1 || sel==2)){printf("Wrong selection(%d)!\nDefault cube will be displayed...Press Enter.\n",sel);sel=1;getch();}

	// Initialize sin/cos tables
	cv_init_trig_tables();
	cv_init_trig_tables_fx();

	// Initialize screen buffer
	screenBuffer = (Byte far*)farmalloc(CV_SCREENRES);
	if(!screenBuffer)
	{
		printf("Error! Not enough memory.");
		exit(1);
	}

	//load test mesh
	if(sel==1)
	{
		if(!cv_mesh_load(&testMesh, "CUBE.OBJ"))
		{
			printf("Error loading obj file!\n");
			goto CV_QUIT;
		}
	} else if(sel==2)
	{
		if(!cv_mesh_load(&testMesh, "SPHERE.OBJ"))
		{
			printf("Error loading obj file!\n");
			goto CV_QUIT;
		}
	}

	transformedFx = (CV_Vec3fx far*)farmalloc(sizeof(CV_Vec3fx)*testMesh.num_verts);
	if(!transformedFx)
	{
		printf(info, "Not enough memory for mesh transform!");
		getch();
		goto CV_QUIT;

	}

	transformed = (CV_Vec3 far*)farmalloc(sizeof(CV_Vec3)*testMesh.num_verts);
	projected   = (CV_Vec2 far*)farmalloc(sizeof(CV_Vec2)*testMesh.num_verts);
	if(!transformed || !projected)
	{
		printf("Not enough memory for mesh...\nPress any key to quit.");getch(); exit(1);
	}

	cv_set_vga_mode();
	cv_make_default_palette();

	// Init main loop
	running=1;
	while(running)
	{
		// Set frameclock
		clock_t now = clock();
		float deltaTime = (now - last_time) / (float)CLOCKS_PER_SEC;
		last_time = now;

		//I/O keyboard
		if(cv_io_key_pressed())
		{
			unsigned char scanCode = cv_io_poll();

			switch(scanCode)
			{
				case KEY_LEFT: angleYf  -= 	5.0f;if(angleYf<0.0f)angleYf=360.0f;break;
				case KEY_RIGHT: angleYf += 	5.0f;if(angleYf>360.0f)angleYf=0.0f;break;
				case KEY_UP: angleXf 	-= 	5.0f;if(angleXf<0.0f)angleXf=360.0f;break;
				case KEY_DOWN: angleXf 	+= 	5.0f;if(angleXf>360.0f)angleXf=0.0f;break;
				case KEY_ESC: running = 0;break;
			}
		}

		angleX = ((int)angleXf) % CV_TABLESIZE;
		angleY = ((int)angleYf) % CV_TABLESIZE;
		angleZ = ((int)angleZf) % CV_TABLESIZE;
		// Clear screen and depth buffer
		_fmemset((Byte far*)screenBuffer, 0, CV_SCREENRES);

		// Transform & project
		for(i=0; i < testMesh.num_verts; ++i)
		{
			//transformed[i] = testMesh.verts[i];
			//cv_rotate(&transformed[i], angleX, angleY, angleZ);
			//projected[i] = cv_project(transformed[i], &mainCam);

			transformedFx[i].x = (fx)(testMesh.verts[i].x * FX_ONE);
			transformedFx[i].y = (fx)(testMesh.verts[i].y * FX_ONE);
			transformedFx[i].z = (fx)(testMesh.verts[i].z * FX_ONE);

			sprintf(info, "tx=%ld\t",transformedFx[i].x);
			cv_draw_text(5,25,info,63,screenBuffer);

			cv_rotate_fx(&transformedFx[i], angleX, angleY, angleZ);

		}

		// Draw tris
		for(i=0; i<testMesh.num_tris; ++i)
		{
			CV_Vec2 v0,v1,v2;
			const int i0=testMesh.tris[i][0], i1=testMesh.tris[i][1], i2=testMesh.tris[i][2];
			int x0,y0,x1,y1,x2,y2,a01x,a02x,a01y,a02y;
			long area2;

			cv_project_fx(transformedFx[i0], &mainCam, &x0, &y0);
			cv_project_fx(transformedFx[i1], &mainCam, &x1, &y1);
			cv_project_fx(transformedFx[i2], &mainCam, &x2, &y2);

			/*a01x = x1 - x0; a01y = y1 - y0;
			a02x = x2 - x0; a02y = y2 - y0;
			area2 = (long)a01x * (long)a02y - (long)a01y * (long)a02x;
			if(area2 <= 0 ) continue;*/

			v0.x = x0; v0.y = y0;
			v1.x = x1; v1.y = y1;
			v2.x = x2; v2.y = y2;

			cv_draw_triangle_fill(&v0, &v1, &v2, 53, screenBuffer);
			cv_draw_triangle_wire(&v0, &v1, &v2, 53, screenBuffer);

			/*CV_Vec3 v0;
			float ux,uy,uz, vx,vy,vz, nx, ny, nz;			// Backface culling
			float dot;
			const int i0 = testMesh.tris[i][0];
			const int i1 = testMesh.tris[i][1];
			const int i2 = testMesh.tris[i][2];

			CV_Vec3 t0 = transformed[i0];
			CV_Vec3 t1 = transformed[i1];
			CV_Vec3 t2 = transformed[i2];

			CV_Vec3 ct;
			ct.x = (t0.x + t1.x + t2.x)/3.0f;
			ct.y = (t0.y + t1.y + t2.y)/3.0f;
			ct.z = (t0.z + t1.z + t2.z)/3.0f;

			ux = t1.x - t0.x; vx = t2.x - t0.x;
			uy = t1.y - t0.y; vy = t2.y - t0.y;
			uz = t1.z - t0.z; vz = t2.z - t0.z;

			nx = uy * vz - uz * vy;
			ny = uz * vx - ux * vz;
			nz = ux * vy - uy * vx;

			v0.x = mainCam.x - ct.x;
			v0.y = mainCam.y - ct.y;
			v0.z = mainCam.z - ct.z;

			dot = nx*v0.x + ny*v0.y + nz*v0.z;

			if(dot<0) continue;

			t0.z = t0.z-mainCam.z;
			t1.z = t1.z-mainCam.z;
			t2.z = t2.z-mainCam.z;

			cv_draw_triangle_fill(&projected[i0], &projected[i1], &projected[i2], 53, screenBuffer);
			cv_draw_triangle_wire(&projected[i0], &projected[i1], &projected[i2], 63, screenBuffer);*/
		}
		// FPS calculations
		if((now - last_fps_clock)>=CLOCKS_PER_SEC)
		{
			fps = frame_count;
			frame_count = 0;
			last_fps_clock = now;
		}

		// Print info on screen
		sprintf(info, "CyberVGA Engine | FPS: %d",fps);
		cv_draw_text(5,5,info,63,screenBuffer);

		frame_count++;

		// Copy screen buffer to vga memory
		_fmemcpy(VGA, screenBuffer, CV_SCREENRES);
	}

	CV_QUIT:
	while(bioskey(1)) bioskey(0);

	cv_set_text_mode();

	farfree(transformed);
	farfree(transformedFx);
	farfree(projected);
	farfree(screenBuffer);
	printf("FPS: %d\nPress any key to exit.");
	getch();
	return 0;
}
