#ifndef CV_SPRIT_H
#define CV_SPRIT_H

#include "CORE\TYPES.h"

typedef struct
{
	u8 manufacturer;
	u8 version;
	u8 encoding;
	u8 bitsPerPixel;
	uint xmin, ymin;
	uint xmax, ymax;
	uint hDPI, vDPI;
	u8 colormap[48];
	u8 reserved;
	u8 nPlanes;
	uint bytesPerLine;
	uint paletteInfo;
	uint hScreenSize, vScreenSize;
	u8 filler[54];

} PCXHeader;

typedef struct
{
	sint width;
	sint height;
	u8 *pixels;
} Sprite;

static void cv_read_le16(const u8 *p, uint *out);
static void cv_parse_pcx(const i8 raw[128], PCXHeader *h);
static int cv_pcx_read_palette(FILE *f, u8 outVga63[256 * 3]);
static int cv_pcx_rle_get(FILE *f, sint *runCount, sint *runValue);

int cv_load_pcx_sprite(const i8 *path,
					   Sprite *outSprite,
					   u8 vgaPalette63[256 * 3]);

#endif