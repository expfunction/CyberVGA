# CyberVGA Engine

MS‑DOS Mode 13h (320×200×8bpp) software 3D and rendering primitives in clean, portable C (C89). CyberVGA targets retro authenticity: it builds with Borland/Turbo C++ 3.x and runs well under DOSBox or on period hardware.

This README reflects the current source layout (CORE, RNDR, IO, MESH, SPRT, GFX, MIDI) and the presence of low‑level VGA assembly support (CV_HW.ASM).

---

## Table of Contents
- Introduction
- Features
- Directory & Architecture
- Getting Started
  - Prerequisites
  - Build (Borland/Turbo C++)
  - Build (OpenWatcom 16‑bit)
  - Assembling CV_HW.ASM
  - Running under DOSBox
- Quickstart (Hello, Mode 13h)
- Module Overview
  - CORE (cybervga core)
  - RNDR (renderer)
  - IO (input, misc I/O)
  - SPRT (sprites)
  - MESH (meshes/models)
  - MIDI (music)
  - GFX (assets)
  - CV_HW.ASM (low‑level VGA)
- Constants, Types, and Memory Model
- Performance Notes
- File Formats
- Roadmap
- Contributing
- License/Attribution

---

## Introduction

CyberVGA is a small, modular engine intended for demoscene‑style effects and early PC game techniques. It wraps the essentials for Mode 13h graphics, palette control, basic math, and a software renderer with an emphasis on clarity and hackability over exotic micro‑optimizations. The code is written in ANSI C (C89/C90) with optional assembly for hotspots.

---

## Features

- VGA Mode 13h initialization/teardown (INT 10h)
- Double‑buffered drawing to an off‑screen framebuffer
- Palette/DAC manipulation (fades, custom palettes)
- Trigonometric lookup tables for fast rotations
- Vector helpers and basic 3D transforms
- Pixel and line rasterization (wireframe primitives)
- Clean headers with a stable `cv_` prefixed API
- Targets Borland/Turbo C++ 3.x; runs under DOSBox

Planned/in progress:
- Keyboard/mouse input, on‑screen text (IO)
- Triangle/polygon fills, spans, texture mapping (RNDR)
- Camera helpers, matrices, additional math utilities
- Optional inline ASM and vertical retrace sync

---

## Directory & Architecture

Top‑level layout:

- CORE/ — Core engine: video mode control, palette, memory, constants
- RNDR/ — Software renderer: pixels, lines, wireframe, future fills/textures
- IO/ — Input and basic I/O (keyboard/mouse, text, HUD) [work in progress]
- SPRT/ — Sprite utilities and blitters
- MESH/ — Mesh/model helpers and sample geometry
- MIDI/ — Music helpers (e.g., simple MIDI playback scaffolding)
- GFX/ — Graphics assets and palettes
- MAIN.C — Example application/game loop that ties modules together
- CV_HW.ASM — Low‑level VGA routines (e.g., DAC I/O, VRAM copies, timing)
- PaletteTechniques.md — Notes and examples for palette effects
- CBE.CVG, CUBE.CVG — Sample engine assets (see File Formats)

Conceptually, an application uses:
- CORE to set graphics mode and manage buffers/palette.
- RNDR for rasterization into a backbuffer.
- IO/SPRT/MESH as needed for input, sprites, and geometry.
- CV_HW.ASM functions where direct VGA port I/O or optimized moves help.

---

## Getting Started

### Prerequisites

- A DOS‑era C compiler:
  - Borland C++ 3.1 (recommended) or Turbo C++ 3.0
  - OpenWatcom 2.x (16‑bit DOS target) — may need small portability shims
- DOSBox (or real DOS hardware/VM) to run the executable

Suggested tools:
- DOSBox 0.74‑3+ (or DOSBox‑X)
- TASM 3.x/5.x if assembling CV_HW.ASM separately

### Build (Borland/Turbo C++)

Command‑line (example; adjust paths):

```bat
REM Large memory model recommended for far pointers
tasm32 /ml CV_HW.ASM       REM assemble low-level VGA helpers -> CV_HW.OBJ

bcc32 -ml -O2 -3 ^
  MAIN.C ^
  CORE\*.C RNDR\*.C IO\*.C SPRT\*.C MESH\*.C ^
  CV_HW.OBJ
```

Borland IDE:
- Options → Compiler:
  - Model: Large
  - Instruction set: 80386 or 80286+
  - Optimize for Speed, Enable Register Vars
- Options → Linker:
  - Produce EXE, map optional
- Add `MAIN.C`, all module `.C` files, and `CV_HW.OBJ` to the project.

### Build (OpenWatcom 16‑bit)

Note: `farmalloc/_fmemcpy/_fmemset` are Borland extensions. If used, either:
- Provide compatibility shims/macros, or
- Replace with Watcom equivalents (e.g., `halloc`, `fmemcpy`, or `memcpy` when safe).

Example:

```bat
wasm /ml CV_HW.ASM           REM or convert ASM syntax as needed for WASM
wcl -ml -bt=dos -ox -s -fe=cybervga.exe ^
  MAIN.C CORE\*.C RNDR\*.C IO\*.C SPRT\*.C MESH\*.C CV_HW.OBJ
```

### Assembling CV_HW.ASM

Typical TASM invocation:

```bat
tasm32 /ml /m2 CV_HW.ASM
```

- `/ml` generates large‑model fixups.
- `/m2` provides # of multiple passes to resolve forward references.
- Outputs `CV_HW.OBJ`, which is linked with the C objects.
- The ASM module exposes small, well‑documented entry points used by CORE/RNDR for VGA port I/O and optimized memory operations.

### Running under DOSBox

1. Copy the built EXE into a directory you will mount in DOSBox.
2. In DOSBox:
   ```bat
   mount c c:\path\to\build
   c:
   cybervga.exe
   ```
3. Exit the demo with any key if BIOS keyboard polling is used.

Tips:
- Set `cycles=auto` or a fixed value (e.g., `cycles=12000`) for consistent timing.
- Palette effects and vsync may vary with DOSBox builds/config.

---

## Quickstart (Hello, Mode 13h)

A minimal loop using the engine’s API:

```c
#include "cybervga.h"
#include "cv_math.h"
#include "cv_renderer.h"

int main(void) {
    Byte far* screenBuffer;

    cv_set_vga_mode();                 /* INT 10h set mode 13h */
    screenBuffer = (Byte far*)farmalloc(CV_SCREENRES);
    if (!screenBuffer) { cv_set_text_mode(); return 1; }

    cv_make_default_palette();         /* Load DAC with a palette */
    cv_init_trig();                    /* Prepare trig lookups */

    while (!bioskey(1)) {              /* Until a key is pressed */
        _fmemset(screenBuffer, 0, CV_SCREENRES);

        /* ... math: rotate, project ... */
        /* ... render: cv_putpixel/cv_draw_line ... */

        _fmemcpy(VGA, screenBuffer, CV_SCREENRES);  /* Flip to VRAM */
    }

    farfree(screenBuffer);
    cv_set_text_mode();                /* Restore text mode */
    return 0;
}
```

Key points:
- Draw into an off‑screen buffer to avoid tearing.
- Blit to `VGA` (0xA000:0000) once per frame.
- Use trig lookup tables for fast animation.

---

## Module Overview

### CORE (cybervga core)

Responsibilities:
- Enter/leave VGA graphics mode
- Create/destroy framebuffers (caller typically owns memory)
- Write VGA palette/DAC (default and custom palettes)
- Provide constants for width/height/pitch and VRAM pointer

Common API (representative):
- `void cv_set_vga_mode(void);`
- `void cv_set_text_mode(void);`
- `void cv_make_default_palette(void);`
- `extern Byte far* VGA; /* 0xA000:0000 */`
- `#define CV_WIDTH  320`
- `#define CV_HEIGHT 200`
- `#define CV_SCREENRES (CV_WIDTH * CV_HEIGHT)`

### RNDR (renderer)

Responsibilities:
- Pixel and line drawing into any CyberVGA‑layout buffer
- Wireframe primitives; foundation for fills and textures

Common API (representative):
- `void cv_putpixel(Byte far* buf, int x, int y, Byte color);`
- `void cv_draw_line(Byte far* buf, int x0, int y0, int x1, int y1, Byte color);`

Implementation notes:
- Integer Bresenham line routine
- Buffer‑agnostic: pass off‑screen buffer or `VGA` directly

### IO (input and I/O) — in progress

Responsibilities:
- Keyboard polling, optional mouse
- Simple text rendering and HUD
- Debug overlay/FPS

### SPRT (sprites)

Responsibilities:
- Sprite formats and blitters
- Transparent/opaque copy, clipping helpers
- Potential palette‑based effects per sprite

### MESH (meshes/models)

Responsibilities:
- Basic geometry structures
- Rotation/projection helpers using cv_math
- Sample models (e.g., CUBE)

### MIDI (music)

Responsibilities:
- Minimal scaffolding around MIDI playback under DOS
- Integration toggles for demos that need audio

### GFX (assets)

- Palettes, images, and other graphics resources used by sample programs.

### CV_HW.ASM (low‑level VGA assembly)

Responsibilities:
- Optimized moves to VGA memory
- Palette DAC port I/O (0x3C8/0x3C9)
- Optional vertical retrace sync (timing‑sensitive)

Integration:
- Built as `CV_HW.OBJ` and linked with the C objects.
- Callable routines are minimal and documented in comments for cross‑compiler friendliness.

---

## Constants, Types, and Memory Model

- Video mode: 320×200, 256 colors (Mode 13h)
- Framebuffer: linear 64,000 bytes (one byte per pixel)
- Memory model: Large (`-ml`) to allow far data and `farmalloc`
- Far pointers: Many routines operate on `Byte far*`
- Palette/DAC: Abstracted writes to ports 0x3C8/0x3C9

Portability:
- Borland `farmalloc`, `_fmemcpy`, `_fmemset` appear in examples.
- Other compilers may need aliases or small substitutions.

---

## Performance Notes

- Prefer backbuffer rendering; single copy to VRAM per frame.
- Keep inner loops free of function call overhead; consider `static inline` where supported by your compiler.
- Fixed‑point or integer math outperforms floats on 16‑bit targets.
- Optional vertical retrace sync reduces tearing but complicates timing.
- DOSBox timing varies with `cycles`; pick a fixed value for consistent playback.

---

## File Formats

- .CVG — Engine assets used by the sample program (e.g., `CBE.CVG`, `CUBE.CVG`). These hold simple data consumed by the demo; inspect the corresponding loader/usage in the source tree for exact structure.

See also:
- Palette techniques and examples: ./PaletteTechniques.md

---

## Roadmap

- RNDR: triangles, spans, solid fills, texture mapping, Z handling
- CORE: palette fades/cycling utilities, LUT helpers
- IO: keyboard + mouse, text, menus, debug HUD
- MESH: camera abstraction, matrix utilities
- ASM: optional routines for hotspots and VSYNC

---

## Contributing

- Keep C89 compatibility; avoid C++ constructs.
- Use the `cv_` prefix for all public functions.
- Keep headers clean and documented; expose only what’s necessary.
- Favor readability; document lookup table scales and units.
- Include testing notes (compiler version, DOSBox config) in pull requests.

---

## License/Attribution

If a LICENSE file is added to the repository, it governs usage. Until then, consider this project all‑rights‑reserved by the author unless explicitly specified otherwise.

---

Happy hacking in glorious 256 colors!