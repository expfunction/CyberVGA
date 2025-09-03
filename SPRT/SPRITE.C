#include <alloc.h>
#include "SPRT\SPRITE.h"

static void cv_read_le16(const u8 *p, uint *out)
{
	*out = (uint)(p[0] | (p[1] << 8));
}

static void cv_parse_pcx(const s8 raw[128], PCXHeader *h)
{
	sint i = 0;
	const u8 *p = raw;
	h->manufacturer = p[0];
	h->version = p[1];
	h->encoding = p[2];
	h->bitsPerPixel = p[3];
	cv_read_le16(p + 4, &h->xmin);
	cv_read_le16(p + 6, &h->ymin);
	cv_read_le16(p + 8, &h->xmax);
	cv_read_le16(p + 10, &h->ymax);
	cv_read_le16(p + 12, &h->hDPI);
	cv_read_le16(p + 14, &h->vDPI);
	for (i = 0; i < 48; ++i)
		h->colormap[i] = p[16 + i];
	h->reserved = p[64];
	h->nPlanes = p[65];
	cv_read_le16(p + 66, &h->bytesPerLine);
	cv_read_le16(p + 68, &h->paletteInfo);
	cv_read_le16(p + 70, &h->hScreenSize);
	cv_read_le16(p + 72, &h->vScreenSize);
	for (i = 0; i < 54; ++i)
		h->filler[i] = p[74 + i];
}

static int cv_pcx_read_palette(FILE *f, u8 outVga63[256 * 3])
{
	u8 raw[256 * 3];
	sint marker, i;
	long cur = ftell(f);

	if (fseek(f, -769, SEEK_END) != 0)
		return 0;
	marker = fgetc(f);
	if (marker != 0x0C)
	{
		fseek(f, cur, SEEK_SET);
		return 0;
	}
	if (fread(raw, 1, 256 * 3, f) != 256 * 3)
	{
		fseek(f, cur, SEEK_SET);
		return 0;
	}
	for (i = 0; i < 256 * 3; i++)
		outVga63[i] = (u8)(raw[i] >> 2);
	fseek(f, cur, SEEK_SET);
	return 1;
}
static sint cv_pcx_rle_get(FILE *f, sint *runCount, sint *runValue)
{
	sint b;
	if (*runCount > 0)
	{
		(*runCount)--;
		return *runValue;
	}
	b = fgetc(f);
	if (b == EOF)
		return -1;
	if ((b & 0xC0) == 0xC0) // RLE
	{
		sint v;
		*runCount = (b & 0x3F);
		v = fgetc(f);
		if (v == EOF)
			return -1;
		*runValue = v;
		(*runCount)--;
		return v;
	}
	else
	{
		*runCount = 0;
		*runValue = b;
		return b;
	}
}

int cv_load_pcx_sprite(const i8 *path,
					   Sprite *outSprite,
					   u8 vgaPalette63[256 * 3])
{
	u8 raw[128];
	u8 *pixels, *scan, *dst;
	uint width, height, x, y;
	PCXHeader h;
	FILE *f = fopen(path, "rb");
	if (!f)
		return 0;

	if (fread(raw, 1, 128, f) != 128)
	{
		fclose(f);
		return 0;
	}

	cv_parse_pcx(raw, &h);

	// Basic checks for 8-bit chunky PCX
	if (h.manufacturer != 0x0A || h.encoding != 1 || h.bitsPerPixel != 8 || h.nPlanes != 1)
	{
		fclose(f);
		return 0;
	}

	width = (uint)(h.xmax - h.xmin + 1);
	height = (uint)(h.ymax - h.ymin + 1);

	pixels = (u8 *)malloc((unsigned long)(width * height));
	if (!pixels)
	{
		fclose(f);
		return 0;
	}

	// Try to read 256-color palette (optional but typical for 8-bit PCX)
	if (!cv_pcx_read_palette(f, vgaPalette63))
	{
		// If missing, you can decide to fail or keep a default palette.
		// Here we keep default (caller may skip palette upload).
	}

	// RLE decode
	if (fseek(f, 128, SEEK_SET) != 0)
	{
		free(pixels);
		fclose(f);
		return 0;
	}

	scan = (u8 *)malloc(h.bytesPerLine);
	if (!scan)
	{
		free(pixels);
		fclose(f);
		return 0;
	}

	for (y = 0; y < height; ++y)
	{
		sint runCount = 0, runValue = 0;
		// Fill scanline up to bytesPerLine
		for (x = 0; x < h.bytesPerLine; ++x)
		{
			sint v = cv_pcx_rle_get(f, &runCount, &runValue);
			if (v < 0)
			{
				free(scan);
				free(pixels);
				fclose(f);
				return 0;
			}
			scan[x] = (unsigned char)v;
		}
		// Copy only visible width (discard padding)
		dst = pixels + (unsigned long)y * width;
		for (x = 0; x < width; ++x)
			dst[x] = scan[x];
	}

	free(scan);
	fclose(f);

	outSprite->width = width;
	outSprite->height = height;
	outSprite->pixels = pixels;
	return 1;
}
