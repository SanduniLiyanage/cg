# HOMEWARD — A Hot-Air Balloon's Journey

A Computer Graphics animated short in legacy/compatibility-profile **OpenGL +
FreeGLUT**. One continuous, ambient scene: a balloon inflates at dawn, drifts
across a living landscape through a full day → sunset → night, then descends
into a city that turns from a flat skyline into a real 3D world. Every required
CG concept is hidden inside the scenery as a natural ingredient (a rope is a DDA
line, the sun is a midpoint circle, parallax is translation, birds leaving the
frame are clipping…). Press **`L`** to reveal faint captions naming the concept
active at each moment — off by default so the watch stays clean.

> Full concept map (all 23 checklist rows → file/function/on-screen moment) is in
> **[CONCEPTS.md](CONCEPTS.md)**.

---

## Build & run on Windows (MSYS2, UCRT64)

Everything below is for the **MSYS2 UCRT64** shell ("MSYS2 UCRT64" from the Start
menu — the prompt says `UCRT64`).

### 1. Install the toolchain & libraries (pacman)

```bash
pacman -Syu          # update first (re-open the shell if it asks you to)

pacman -S --needed \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-ninja \
    mingw-w64-ucrt-x86_64-freeglut \
    mingw-w64-ucrt-x86_64-glm
```

* `gcc` — the MinGW-w64 C++ compiler (GLU/OpenGL import libs ship with it).
* `cmake`, `ninja` — build system + generator.
* `freeglut` — the windowing/`glut` layer.
* `glm` — optional matrix maths (the project hand-builds its matrices, so this is
  not strictly required; CMake just enables `HAVE_GLM` if it's present).

### 2. Configure & build

```bash
cd cg      # this repo
cmake -S . -B build -G Ninja
cmake --build build
```

### 3. Run

```bash
./build/homeward.exe
```

Optional verification arguments: `./build/homeward.exe <startSeconds> [pause] [labels]`
— e.g. `./build/homeward.exe 188 pause labels` jumps to the night city, pauses,
and shows the concept captions.

> If you launch the `.exe` from outside the UCRT64 shell, make sure
> `C:\msys64\ucrt64\bin` is on your `PATH` so it can find `libfreeglut.dll` etc.

---

## Controls

| Key | Action |
|---|---|
| `L` | Toggle faint concept captions (also shows the clip window + software Z-buffer panel) |
| `Space` | Pause / resume |
| `0`–`5` | Jump to segment 0–5 |
| `←` / `→` | Scrub 3 s back / forward |
| `Esc` | Quit |

The film **auto-plays** start→finish (~230 s) and **loops** gently.

---

## The continuous timeline

| Seg | Time | What plays |
|---|---|---|
| 0 | 0:00–0:25 | Pre-dawn: starfield, balloon inflates, tether ropes |
| 1 | 0:25–1:00 | Dawn liftoff: sky warms, sun arcs up, parallax world scrolls, lake reflection |
| 2 | 1:00–1:45 | Daytime drift: clouds, birds clipped at the frame, windmill, flags, power lines |
| 3 | 1:45–2:25 | Golden hour: long sheared shadows, slanted rain, a shooting star |
| 4 | 2:25–3:20 | Descent: flat skyline (parallel) → perspective city, orbit, depth |
| 5 | 3:20–3:50 | Touchdown: lanterns, moon, hold, then loop |

---

## Project layout (matches the spec)

```
cg/
├── CMakeLists.txt
├── src/
│   ├── main.cpp                 # window, timeline clock, orchestration, L-toggle, HUD
│   ├── common/                  # shared types + the single SceneState
│   ├── m1_primitives/           # M1: DDA, Bresenham, midpoint circle, points/poly
│   ├── m2_fills_transforms/     # M2: scan-line/boundary/flood fill + 2D transforms + 2D world
│   ├── m3_camera_clipping/      # M3: window->viewport, Cohen-Sutherland, Liang-Barsky
│   ├── m4_city_3d/              # M4: 3D city, parallel<->perspective morph, orbit
│   └── m5_depth_timeline/       # M5: keyframe timeline, depth test/cull, software Z-buffer
└── README.md / CONCEPTS.md
```

## Member ownership (each owns a *system*, not a slide)

* **M1 — The Draftsman** (`m1_primitives/`): points, lines (DDA/Bresenham), midpoint circle, polygons.
* **M2 — The Painter** (`m2_fills_transforms/`): boundary/flood/scan-line fill + all 2D transforms + the 2D world.
* **M3 — The Camera** (`m3_camera_clipping/`): window→viewport mapping, Cohen–Sutherland, Liang–Barsky.
* **M4 — The Architect** (`m4_city_3d/`): the 3D city, parallel & perspective projection, camera orbit, 3D transforms.
* **M5 — The Director** (`m5_depth_timeline/`): hidden-surface removal, Z-buffer (hardware + software), keyframe timeline.

## What is hand-coded vs. delegated to OpenGL

**Hand-implemented** (pixels computed in our code, plotted with
`glBegin(GL_POINTS); glVertex2i; glEnd();`): DDA, Bresenham, midpoint circle,
boundary fill, flood fill, scan-line fill, Cohen–Sutherland, Liang–Barsky — plus
a bonus **software Z-buffer** (two overlapping triangles resolved with our own
depth array, shown in the HUD when `L` is on).

**OpenGL built-ins (algorithm explained verbally):** the 3D transforms
(`glTranslatef/glRotatef/glScalef`, `gluLookAt`), projections (hand-built
ortho⇄perspective matrices loaded with `glLoadMatrixf`, plus `gluPerspective`
maths), hidden-surface removal (`glEnable(GL_CULL_FACE)`), and the hardware
Z-buffer (`glEnable(GL_DEPTH_TEST)`).
