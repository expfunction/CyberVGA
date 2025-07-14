# CyberVGA: VGA Palette Manipulation Techniques

## Introduction

This document describes how palette manipulation works in VGA Mode 13h and how these retro techniques can be used to achieve visual effects similar to modern shaders—using only palette animation and color mapping.

---

## VGA Palette Basics

* **Mode 13h (320x200, 256 colors)** uses an indexed color palette.
* Each pixel is a **byte (0-255)**, serving as an **index** into a palette of 256 colors.
* The VGA palette is stored in hardware: **256 colors x 3 bytes** (R, G, B), with each component 0-63 (6 bits).

### Palette RAM and Registers

* The palette resides in the VGA's RAMDAC.
* VGA registers:

  * **0x3C8**: Palette index port (write index to set)
  * **0x3C9**: Palette data port (write R, G, B, 0-63)

### Setting Palette Colors

To set palette entry N:

```
outp(0x3C8, N);    // Select palette index N
outp(0x3C9, R);     // Red (0-63)
outp(0x3C9, G);     // Green (0-63)
outp(0x3C9, B);     // Blue (0-63)
```

To set all 256 entries:

```
outp(0x3C8, 0); // Start from color 0
for(i=0; i<256; ++i) {
    outp(0x3C9, r[i]);
    outp(0x3C9, g[i]);
    outp(0x3C9, b[i]);
}
```

---

## Palette-Based Visual Effects

### Palette Cycling ("Color Cycling")

* By rotating the RGB values of palette entries (for a range of indices), you create the illusion of motion (e.g., water, fire, rainbow) **with no changes to screen memory.**
* Example: Animate colors 32-47 for a flowing water effect by shifting their palette entries each frame.

### Palette Fades (Fade In/Out)

* Interpolate all palette entries between black and their intended color to fade in, or to black to fade out.
* Achieve scene fade, flashes, or health effects with just palette changes.

### Pseudo-Lighting and "Retro Shaders"

* Draw gradients or objects using a range of color indices (e.g., 8=dark, 12=mid, 15=bright).
* Animate the palette entries for those indices to simulate light changes, pulses, or glows.
* You can fake highlights, environment effects, and more—like a retro fragment shader!

### Palette-Mapped Textures

* Sprites and textures can use indices mapped to a palette. By changing the palette, you instantly recolor the whole texture.
* Classic for recoloring player sprites, animated effects, or day/night transitions.

### Why 0-63 and not 0-255 for RGB?

* VGA hardware uses 6 bits per channel (0-63).
* Even though the RAMDAC outputs analog, the register interface is only 6 bits wide.

---

## Advanced Examples

* **Water/Fire/Plasma:**

  * Draw the shape or pattern using a block of palette indices.
  * Cycle just those palette entries for movement.
* **Palette-Based Fades:**

  * Step all entries toward black (0,0,0) or white (63,63,63) for fading.
* **Fake Lighting:**

  * Reserve indices for "highlight" or "shadow" bands; change palette entries to brighten or dim entire scenes instantly.

---

## Summary Table

| Effect            | How it's Done               |
| ----------------- | --------------------------- |
| Palette cycling   | Shift palette entries       |
| Fade in/out       | Step palette RGB values     |
| Pseudo-lighting   | Animate palette for indices |
| Texture recolor   | Change palette used by tex  |
| Water/Plasma/Fire | Cycle colors in a band      |

---

## Takeaway

* Palette manipulation is the "shader" of the VGA era.
* Allows for full-scene effects and efficient, beautiful visuals in retro games and demos.
* **You can animate, recolor, fade, and light a scene—just by changing palette entries.**

---

# Next: See the CyberVGA API for palette routines and sample usage.
