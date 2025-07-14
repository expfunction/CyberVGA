\# CyberVGA Engine: Technical Overview



\## Introduction



CyberVGA is a modular, retro-style 3D graphics engine targeting MS-DOS and VGA Mode 13h (320x200, 256 colors). Inspired by classic demoscene and early PC game engines, CyberVGA provides clean C interfaces for 3D math, rendering, palette manipulation, and soon, input handling. This document outlines the engine architecture, modules, and design choices for easy expansion and maintenance.



---



\## Architecture



CyberVGA is organized into the following modules:



\* \*\*cybervga\*\*: Core engine API, video mode control, palette routines, memory and constants.

\* \*\*cv\\\_math\*\*: 3D math (vectors, rotation, projection), trigonometric tables, coordinate transforms.

\* \*\*cv\\\_renderer\*\*: Software rasterization, lines, pixels, wireframe objects, future fills.

\* \*\*cv\\\_io\*\*: (planned) Input handling: keyboard, mouse, simple text, etc.

\* \*\*main.c\*\*: High-level application code (game loop, object data, engine calls).



Each module is defined by a header (`.h`) and implementation (`.c`) file.



---



\## Module Details



\### cybervga (core)



\* Sets VGA graphics and text mode.

\* Allocates/destroys framebuffers.

\* Palette manipulation API.

\* Engine-wide constants (width, height, etc).



\### cv\\\_math



\* Defines 3D and 2D vector types.

\* Manages trigonometric lookup tables.

\* Provides rotation and perspective projection.

\* Extensible for additional math (matrix, scaling, etc).



\### cv\\\_renderer



\* Pixel and line drawing routines for framebuffers.

\* Accepts any buffer conforming to CyberVGA's layout.

\* Intended for wireframes, raster objects, and future solid/texture fills.



\### cv\\\_io (planned)



\* Input polling for keyboard (and mouse, future).

\* Simple on-screen text rendering, menu navigation.

\* Optionally: debug overlay, FPS display.



---



\## Engine Flow Example



```c

cv\_set\_vga\_mode();

screenBuffer = (Byte far\*)farmalloc(CV\_SCREENRES);

cv\_make\_default\_palette();

cv\_init\_trig();



// Main loop

while (!bioskey(1)) {

&nbsp;   \_fmemset(screenBuffer, 0, CV\_SCREENRES);

&nbsp;   // ... math, rotation, projection ...

&nbsp;   // ... draw cube/wireframe with cv\_draw\_line() ...

&nbsp;   \_fmemcpy(VGA, screenBuffer, CV\_SCREENRES);

}

farfree(screenBuffer);

cv\_set\_text\_mode();

```



---



\## Coding Standards \& Style



\* Standard C89/C90 (Borland C++ 3.1 compatible).

\* Use `typedef struct` for vectors, points, etc.

\* All module functions use `cv\_` prefix for namespace safety.

\* Expose only necessary data/functions via headers.

\* Avoid global variables unless strictly needed (e.g. static trig tables).

\* Avoid C++ features to maximize DOS C compatibility.

\* Clean function prototypes and documentation in headers.



---



\## Extension Points



\* \*\*Rendering\*\*: Add triangle, polygon, texture-mapping routines to `cv\_renderer`.

\* \*\*Math\*\*: Add matrix, camera, physics helpers to `cv\_math`.

\* \*\*Palette\*\*: Add fades, cycles, LUTs to `cybervga`.

\* \*\*I/O\*\*: Add menu/input logic in `cv\_io`.

\* \*\*Optimizations\*\*: Inline asm for speed, vsync, double buffering tweaks.



---



\## Example Application Structure



```

main.c

cybervga.c/h   // Core: mode set, palette, constants

cv\_math.c/h    // Math: vectors, rotation, projection

cv\_renderer.c/h// Render: pixel, line, cube drawing

cv\_io.c/h      // I/O: input, text, menu (future)

```



---



\## Philosophy



\* \*\*Simplicity first:\*\* Easy-to-follow functions, minimal global state.

\* \*\*Retro authenticity:\*\* All code compiles and runs on 1990s DOS (Borland/Turbo C).

\* \*\*Maximum hackability:\*\* Add, swap, or replace modules with minimal friction.

\* \*\*Clear C separation:\*\* Each module is easy to test, use, or replace.



---



\## See also



\* \\\[CyberVGA Palette Techniques]

\* (Planned) Math/Render/I-O API reference



---



\# Next: Expand each module's documentation and add usage examples.



