/* cvg1.h — C89, Borland C++ 4.5 + PowerPack */
#include "CORE/TYPES.h"

#ifndef CVG1_H
#define CVG1_H

#ifdef __cplusplus
extern "C"
{
#endif
/* Flags */
#define CVG_HAS_UV 0x01
#define CVG_HAS_NORMALS 0x02
#define CVG_HAS_VCOLORS 0x04

    typedef struct MeshCVG1
    {
        u16 vertex_count;
        u16 index_count;
        u16 submesh_count;

        i32 *pos_fx;       /* v*3, 16.16 */
        u32 *uv_fx;        /* v*2, 16.16 (or NULL) */
        i8 *normals;       /* v*3 (or NULL) */
        u8 *vcolors;       /* v (or NULL) */
        u16 *indices;      /* i */
        u16 *submesh_data; /* sm*4 (first,count,mat,0) */

        u8 flags;
        i32 scale_16_16;
    } MeshCVG1;

    /* API */
    int load_cvg1(const char *path, MeshCVG1 *out_mesh); /* returns 1 on success */
    void free_mesh(MeshCVG1 *m);                         /* frees allocations */

#ifdef __cplusplus
}
#endif
#endif /* CVG1_H */
