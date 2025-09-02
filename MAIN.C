#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "mesh\cvg1.h"
#include "core\fixed.h"
#include "gfx\vga.h"
#include "rndr\wire.h"
#include "rndr\camer.h"

int main(int argc, char **argv)
{
	MeshCVG1 mesh;
	Surface8 surf = {320, 200, 0};
	i32 angle_y = 0, angle_x = 0;
	i32 rot_y = FX_FROM_INT(0); /* set below */
	i32 rot_x = FX_FROM_INT(0);
	i32 f_fx = FX_FROM_INT(220);
	/* Speeds (Q16.16) */
	i32 move_step = FX_FROM_INT(1) / 32;          /* ~0.03125 units per key press */
	i32 rot_step = fx_deg_to_rad(FX_FROM_INT(2)); /* 2 degrees per tap */
	int running = 1, key;
	int *sx = 0, *sy = 0;
	i32 *tx = 0, *ty = 0, *tz = 0;
	Camera cam;
	CamBasis cb;

	if (argc < 2)
	{
		printf("use: VIEW model.cvg\n");
		return 1;
	}
	memset(&mesh, 0, sizeof(mesh));
	if (!load_cvg1(argv[1], &mesh))
	{
		printf("load fail\n");
		return 1;
	}

	/* Setup camera */
	cam.pos_x = FX_FROM_INT(0);
	cam.pos_y = FX_FROM_INT(0);
	cam.pos_z = FX_FROM_INT(-3); /* camera 3 units back looking toward +Z */
	cam.yaw = 0;
	cam.pitch = 0;
	cam.roll = 0;
	cam.znear = FX_FROM_INT(1);
	cam.f_fx = cam_focal_from_fov_deg(320, (i32)(60 * 65536L)); /* ~60° FOV */

	fixed_init_trig();
	rot_y = FX_FROM_INT(2) / 128; /* ~0.0156 rad/frame */
	rot_x = FX_FROM_INT(1) / 128;

	surf.back = (unsigned char *)malloc(64000UL);
	if (!surf.back)
	{
		free_mesh(&mesh);
		return 1;
	}
	sx = (int *)malloc(sizeof(int) * mesh.vertex_count);
	sy = (int *)malloc(sizeof(int) * mesh.vertex_count);
	tx = (i32 *)malloc(sizeof(i32) * mesh.vertex_count);
	ty = (i32 *)malloc(sizeof(i32) * mesh.vertex_count);
	tz = (i32 *)malloc(sizeof(i32) * mesh.vertex_count);
	if (!sx || !sy || !tx || !ty || !tz)
	{
		return 1;
	}

	vga_set_mode13h();

	while (running)
	{
		u32 v;
		i32 siny = fx_sin(angle_y), cosy = fx_cos(angle_y);
		i32 sinx = fx_sin(angle_x), cosx = fx_cos(angle_x);

		cam_build_basis(&cam, &cb);

		wire_clear(&surf, 0);
		back_draw_border(surf.back); /* prove blit path */

		/* ----- INPUT (non-blocking, pure DOS) ----- */
		/* We consume all buffered keys this frame so holds repeat smoothly */
		key;
        while (kbhit())
        {
            key = getch();

            /* Extended keys come as 0 or 224, then a second code */
            if (key == 0 || key == 224)
            {
                int k2 = getch();
                switch (k2)
                {
                case 75: /* Left  */
                    cam.yaw -= rot_step;
                    break;
                case 77: /* Right */
                    cam.yaw += rot_step;
                    break;
                case 72: /* Up    */
                    cam.pitch -= rot_step;
                    break;
                case 80: /* Down  */
                    cam.pitch += rot_step;
                    break;
                default:
                    break;
                }
                continue;
            }

            /* ASCII keys (WASD QE, roll with , . ; adjust speed with - +) */
            switch (key)
            {
            case 27:
                running = 0;
                break; /* ESC */

            case 'w':
            case 'W':
                cam_move_local(&cam, &cb, 0, 0, +move_step);
                break; /* forward */
            case 's':
            case 'S':
                cam_move_local(&cam, &cb, 0, 0, -move_step);
                break; /* back    */
            case 'a':
            case 'A':
                cam_move_local(&cam, &cb, -move_step, 0, 0);
                break; /* strafe L*/
            case 'd':
            case 'D':
                cam_move_local(&cam, &cb, +move_step, 0, 0);
                break; /* strafe R*/
            case 'q':
            case 'Q':
                cam_move_local(&cam, &cb, 0, +move_step, 0);
                break; /* up      */
            case 'e':
            case 'E':
                cam_move_local(&cam, &cb, 0, -move_step, 0);
                break; /* down    */

            case ',':
                cam.roll -= rot_step;
                break;
            case '.':
                cam.roll += rot_step;
                break;

            case '-': /* slower movement */
                if (move_step > 512)
                    move_step >>= 1;
                break;
            case '+':
            case '=': /* faster movement */
                if (move_step < FX_FROM_INT(4))
                    move_step <<= 1;
                break;

            default:
                break;
            }
        }
        /* keep pitch sane */
        cam_clamp_pitch(&cam);

        for (v = 0; v < (u32)mesh.vertex_count; ++v)
        {
            i32 wx = mesh.pos_fx[v * 3 + 0];
            i32 wy = mesh.pos_fx[v * 3 + 1];
            i32 wz = mesh.pos_fx[v * 3 + 2];

            i32 vx, vy, vz;
            cam_world_to_view(&cb, wx, wy, wz, cam.pos_x, cam.pos_y, cam.pos_z, &vx, &vy, &vz);

            /* Skip points behind near plane */
            if (vz < cam.znear)
            {
                tz[v] = cam.znear;
                tx[v] = vx;
                ty[v] = vy;
            }
            else
            {
                tx[v] = vx;
                ty[v] = vy;
                tz[v] = vz;
            }
        }
        for (v = 0; v < (u32)mesh.vertex_count; ++v)
            wire_project16_16(tx[v], ty[v], tz[v], cam.f_fx, &sx[v], &sy[v]);

        wire_draw_mesh_edges_culled(&mesh, sx, sy, &surf, 15, /*ccw_front=*/0);

        // vsync_wait();  /* optional */
        vga_blit_8(surf.back); /* ALWAYS blit */

        if (kbhit())
        {
            int c = getch();
            if (c == 27)
                running = 0;
        }
    }

    vga_reset_text();
    free(sx);
    free(sy);
    free(tx);
    free(ty);
    free(tz);
    free(surf.back);
    free_mesh(&mesh);
    return 0;
}
