
/* cvg1_loader.c - C89, Borland C++ 4.5 + PowerPack */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mesh\cvg1.h"
int read_exact(void *dst, size_t size, size_t count, FILE *fp)
{
    return (fread(dst, size, count, fp) == count) ? 1 : 0;
}

void free_mesh(MeshCVG1 *m)
{
    if (!m)
        return;
    if (m->pos_fx)
        free(m->pos_fx);
    if (m->uv_fx)
        free(m->uv_fx);
    if (m->normals)
        free(m->normals);
    if (m->vcolors)
        free(m->vcolors);
    if (m->indices)
        free(m->indices);
    if (m->submesh_data)
        free(m->submesh_data);
    memset(m, 0, sizeof(*m));
}

/* Returns 1 on success, 0 on failure */
int load_cvg1(const char *path, MeshCVG1 *out_mesh)
{
    FILE *fp;
    u8 magic[4];
    u16 version;
    u8 flags, coord_fmt;
    u16 vcount, icount, scount, reserved16;
    u32 ofs_vertices, ofs_indices, ofs_uv0, ofs_normals, ofs_vcolors, ofs_submeshes;
    i32 scale_16_16;
    u32 reserved32;
    long base;

    /* temp buffers for raw reads */
    i16 *raw_pos = 0;
    u16 *raw_uv = 0;
    i8 *raw_n = 0;
    u8 *raw_c = 0;

    MeshCVG1 m;
    memset(&m, 0, sizeof(m));

    fp = fopen(path, "rb");
    if (!fp)
        return 0;

    base = ftell(fp);

    if (!read_exact(magic, 1, 4, fp))
    {
        fclose(fp);
        return 0;
    }
    if (magic[0] != 'C' || magic[1] != 'V' || magic[2] != 'G' || magic[3] != '1')
    {
        fclose(fp);
        return 0;
    }

    if (!read_exact(&version, 2, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&flags, 1, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&coord_fmt, 1, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&vcount, 2, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&icount, 2, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&scount, 2, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&reserved16, 2, 1, fp))
    {
        fclose(fp);
        return 0;
    }

    if (!read_exact(&ofs_vertices, 4, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&ofs_indices, 4, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&ofs_uv0, 4, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&ofs_normals, 4, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&ofs_vcolors, 4, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&ofs_submeshes, 4, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&scale_16_16, 4, 1, fp))
    {
        fclose(fp);
        return 0;
    }
    if (!read_exact(&reserved32, 4, 1, fp))
    {
        fclose(fp);
        return 0;
    }

    if (version != 1)
    {
        fclose(fp);
        return 0;
    }
    if (coord_fmt != 0)
    {
        fclose(fp);
        return 0;
    }
    if (vcount == 0 || icount == 0 || scount == 0)
    {
        fclose(fp);
        return 0;
    }
    if ((icount % 3) != 0u)
    {
        fclose(fp);
        return 0;
    }

    /* allocate runtime arrays */
    m.vertex_count = vcount;
    m.index_count = icount;
    m.submesh_count = scount;
    m.flags = flags;
    m.scale_16_16 = scale_16_16;

    m.pos_fx = (i32 *)malloc(sizeof(i32) * 3u * vcount);
    if (!m.pos_fx)
    {
        fclose(fp);
        return 0;
    }

    m.indices = (u16 *)malloc(sizeof(u16) * icount);
    if (!m.indices)
    {
        free_mesh(&m);
        fclose(fp);
        return 0;
    }

    m.submesh_data = (u16 *)malloc(sizeof(u16) * 4u * scount);
    if (!m.submesh_data)
    {
        free_mesh(&m);
        fclose(fp);
        return 0;
    }

    if (flags & CVG_HAS_UV)
    {
        m.uv_fx = (u32 *)malloc(sizeof(u32) * 2u * vcount);
        if (!m.uv_fx)
        {
            free_mesh(&m);
            fclose(fp);
            return 0;
        }
    }
    if (flags & CVG_HAS_NORMALS)
    {
        m.normals = (i8 *)malloc(sizeof(i8) * 3u * vcount);
        if (!m.normals)
        {
            free_mesh(&m);
            fclose(fp);
            return 0;
        }
    }
    if (flags & CVG_HAS_VCOLORS)
    {
        m.vcolors = (u8 *)malloc(sizeof(u8) * vcount);
        if (!m.vcolors)
        {
            free_mesh(&m);
            fclose(fp);
            return 0;
        }
    }

    /* read raw vertices (int16) then convert to 16.16 */
    if (fseek(fp, (long)ofs_vertices, SEEK_SET) != 0)
    {
        free_mesh(&m);
        fclose(fp);
        return 0;
    }
    raw_pos = (i16 *)malloc(sizeof(i16) * 3u * vcount);
    if (!raw_pos)
    {
        free_mesh(&m);
        fclose(fp);
        return 0;
    }
    if (!read_exact(raw_pos, sizeof(i16) * 3u, vcount, fp))
    {
        free(raw_pos);
        free_mesh(&m);
        fclose(fp);
        return 0;
    }

    {
        u32 i, k;
        i32 s = scale_16_16;
        k = 0u;
        for (i = 0u; i < (u32)vcount; ++i)
        {
            /* result is 16.16 fixed: int32(x_raw) * scale_16_16 */
            m.pos_fx[k + 0] = ((i32)raw_pos[k + 0]) * s;
            m.pos_fx[k + 1] = ((i32)raw_pos[k + 1]) * s;
            m.pos_fx[k + 2] = ((i32)raw_pos[k + 2]) * s;
            k += 3u;
        }
    }
    free(raw_pos);
    raw_pos = 0;

    /* indices */
    if (fseek(fp, (long)ofs_indices, SEEK_SET) != 0)
    {
        free_mesh(&m);
        fclose(fp);
        return 0;
    }
    if (!read_exact(m.indices, sizeof(u16), icount, fp))
    {
        free_mesh(&m);
        fclose(fp);
        return 0;
    }

    /* UV */
    if ((flags & CVG_HAS_UV) && ofs_uv0 != 0u)
    {
        u32 i;
        raw_uv = (u16 *)malloc(sizeof(u16) * 2u * vcount);
        if (!raw_uv)
        {
            free_mesh(&m);
            fclose(fp);
            return 0;
        }
        if (fseek(fp, (long)ofs_uv0, SEEK_SET) != 0)
        {
            free(raw_uv);
            free_mesh(&m);
            fclose(fp);
            return 0;
        }
        if (!read_exact(raw_uv, sizeof(u16) * 2u, vcount, fp))
        {
            free(raw_uv);
            free_mesh(&m);
            fclose(fp);
            return 0;
        }
        for (i = 0u; i < (u32)vcount; ++i)
        {
            u32 uu = (u32)raw_uv[i * 2u + 0];
            u32 vv = (u32)raw_uv[i * 2u + 1];
            /* to 16.16: (val << 16) / 65535 */
            m.uv_fx[i * 2u + 0] = (uu << 16) / 65535u;
            m.uv_fx[i * 2u + 1] = (vv << 16) / 65535u;
        }
        free(raw_uv);
        raw_uv = 0;
    }

    /* Normals */
    if ((flags & CVG_HAS_NORMALS) && ofs_normals != 0u)
    {
        if (fseek(fp, (long)ofs_normals, SEEK_SET) != 0)
        {
            free_mesh(&m);
            fclose(fp);
            return 0;
        }
        /* stored as 4 bytes per vertex: nx,ny,nz,pad */
        {
            u32 bytes = (u32)vcount * 4u;
            u8 *buf = (u8 *)malloc(bytes);
            u32 i;
            if (!buf)
            {
                free_mesh(&m);
                fclose(fp);
                return 0;
            }
            if (!read_exact(buf, 1, bytes, fp))
            {
                free(buf);
                free_mesh(&m);
                fclose(fp);
                return 0;
            }
            for (i = 0u; i < (u32)vcount; ++i)
            {
                m.normals[i * 3u + 0] = (i8)buf[i * 4u + 0];
                m.normals[i * 3u + 1] = (i8)buf[i * 4u + 1];
                m.normals[i * 3u + 2] = (i8)buf[i * 4u + 2];
            }
            free(buf);
        }
    }

    /* Vertex colors */
    if ((flags & CVG_HAS_VCOLORS) && ofs_vcolors != 0u)
    {
        if (fseek(fp, (long)ofs_vcolors, SEEK_SET) != 0)
        {
            free_mesh(&m);
            fclose(fp);
            return 0;
        }
        {
            u32 bytes = (u32)vcount * 4u; /* idx + 3 pads */
            u8 *buf = (u8 *)malloc(bytes);
            u32 i;
            if (!buf)
            {
                free_mesh(&m);
                fclose(fp);
                return 0;
            }
            if (!read_exact(buf, 1, bytes, fp))
            {
                free(buf);
                free_mesh(&m);
                fclose(fp);
                return 0;
            }
            for (i = 0u; i < (u32)vcount; ++i)
            {
                m.vcolors[i] = buf[i * 4u + 0];
            }
            free(buf);
        }
    }

    /* Submeshes: 4 u16 each */
    if (fseek(fp, (long)ofs_submeshes, SEEK_SET) != 0)
    {
        free_mesh(&m);
        fclose(fp);
        return 0;
    }
    if (!read_exact(m.submesh_data, sizeof(u16) * 4u, scount, fp))
    {
        free_mesh(&m);
        fclose(fp);
        return 0;
    }

    fclose(fp);
    *out_mesh = m;
    return 1;
}

/* Example tiny test */
#ifdef TEST_CVG1
#include <assert.h>
int main(int argc, char **argv)
{
    MeshCVG1 m;
    int ok;
    if (argc < 2)
    {
        printf("usage: %s file.cvg\n", argv[0]);
        return 0;
    }
    memset(&m, 0, sizeof(m));
    ok = load_cvg1(argv[1], &m);
    if (!ok)
    {
        printf("load failed\n");
        return 1;
    }
    printf("verts=%u idx=%u submeshes=%u flags=0x%02X scale=%ld\n",
           (unsigned)m.vertex_count, (unsigned)m.index_count, (unsigned)m.submesh_count,
           (unsigned)m.flags, (long)m.scale_16_16);
    free_mesh(&m);
    return 0;
}
#endif
