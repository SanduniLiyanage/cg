# HOMEWARD — Concept coverage report

All 23 rows of the spec's concept checklist, mapped to the exact file + function
and the on-screen moment where each appears. Press **`L`** in the running program
to see the matching captions live.

Legend: **Hand** = pixel-level algorithm we compute ourselves and plot with
`glBegin(GL_POINTS)`. **GL** = OpenGL built-in, algorithm explained verbally.

| # | Concept | Owner | Seg | File · function | On-screen moment | Kind |
|---|---|---|---|---|---|---|
| 1 | Points, lines, circles, polygons | M1 | all | `m1_primitives/primitives.cpp` · `pointStar`, `ddaLine`, `bresenhamLine`, `midpointCircle`, `polylineDDA` | Stars, ropes, sun/moon, mountains, buildings | Hand |
| 2 | DDA line algorithm | M1 | 0 | `primitives.cpp` · `ddaLine` (called in `world2d.cpp` · `drawBalloon`) | Balloon tether ropes (real diagonal slope) | Hand |
| 3 | Bresenham line algorithm | M1 | 2 | `primitives.cpp` · `bresenhamLine` (in `world2d.cpp` · `drawPowerLines`, `drawMountains`) | Sagging power lines & mountain ridges | Hand |
| 4 | Midpoint circle algorithm | M1 | 1,5 | `primitives.cpp` · `midpointCircle` | Sun & moon outline, balloon envelope, lanterns | Hand |
| 5 | Boundary fill | M2 | 1 | `m2_fills_transforms/fills.cpp` · `markCircleBoundary` + `boundaryFill4` (in `world2d.cpp` · `drawSun`/`drawBalloon`) | Sun disc & balloon envelope filled to their circular border | Hand |
| 6 | Flood fill | M2 | 1 | `fills.cpp` · `floodFill4` (in `world2d.cpp` · `initWorld2D`/`drawLake`) | The lake water region | Hand |
| 7 | Scan-line fill | M2 | 1,2 | `fills.cpp` · `scanlineFill` | Triangular mountains, rooftops, pennant flags, windmill blades, basket | Hand |
| 8 | Translation | M2 | 1,2 | `transforms.cpp` · `translateP`/`translate`; parallax via `world2d.cpp` layer offsets & `drawClouds` | Parallax layers at different speeds; cloud/balloon drift | Hand |
| 9 | Rotation | M2 | 2 | `transforms.cpp` · `rotateP`/`rotate` (in `world2d.cpp` · `drawWindmill`); balloon sway in `drawBalloon` | Windmill blades turning; balloon swaying | Hand |
| 10 | Scaling | M2 | 0,4 | `transforms.cpp` · `scaleP`; balloon `R*inflate` in `drawBalloon`; city `glScalef(cityScale)` | Balloon inflation; city growing into view | Hand (2D) / GL (3D) |
| 11 | Reflection | M2 | 1 | `transforms.cpp` · `reflectY` (axis = waterline); lake mirror in `world2d.cpp` · `drawLake` | Balloon & sky mirrored in the lake (scissor-clipped strip) | Hand |
| 12 | Shearing | M2 | 3 | `transforms.cpp` · `shearX` (in `world2d.cpp` · `drawBalloonShadow` & `drawFarmhouse`) | Long leaning shadows; pennant flags bent by wind | Hand |
| 13 | Window, viewport & mapping | M3 | 2 | `main.cpp` · `set2DWindow` (`gluOrtho2D` moving window + `glViewport`); `camera.cpp` · `windowToViewport` | The camera window scrolling over the wide world | GL + Hand |
| 14 | Cohen–Sutherland clipping | M3 | 2 | `camera.cpp` · `clipCS` (in `drawClipped` · `birdSeg`) | Birds cleanly cut at the rectangular frame border | Hand |
| 15 | Liang–Barsky clipping | M3 | 3 | `camera.cpp` · `clipLB` (in `drawClipped` · `rainSeg`) | Slanted rain streaks & the shooting star trimmed at the border | Hand |
| 16 | 3D translation | M4 | 4 | `city.cpp` · `drawCity` · `glTranslatef(0,(cityRise-1)*45,0)` | The city rising into place from below | GL |
| 17 | 3D rotation | M4 | 4 | `city.cpp` · `drawCity` · `gluLookAt` with `camOrbit` | The camera orbiting the city | GL |
| 18 | 3D scaling | M4 | 4 | `city.cpp` · `drawCity` · `glScalef(cityScale,…)` | The city growing from distant to full view | GL |
| 19 | Parallel projection | M4 | 4 | `city.cpp` · `orthoM` (blended at `cityReveal=0`) | The far, flat, map-like skyline (no foreshortening) | GL/Hand matrix |
| 20 | Perspective projection | M4 | 4 | `city.cpp` · `perspM` (blended at `cityReveal=1`) | In-street descent with a vanishing point | GL/Hand matrix |
| 21 | Hidden surface removal | M5 | 4 | `city.cpp` · `drawCity` · `glEnable(GL_CULL_FACE)` (CCW faces in `drawBuilding`) | Solid buildings — their backs are never seen | GL |
| 22 | Z-buffer algorithm | M5 | 4 | `glEnable(GL_DEPTH_TEST)` in `city.cpp`; software proof in `softzbuffer.cpp` · `drawSoftZBufferDemo` | Towers + balloon overlap resolved per pixel; software demo in HUD with `L` | GL + Hand (bonus) |
| 23 | Keyframes & interpolation | M5 | all | `m5_depth_timeline/timeline.cpp` · `updateTimeline` + `timeline.h` · `Track::sample` | The whole day-night journey (sky, sun, balloon, camera) | Hand |

## Notes for the examiner

* The eight pixel-level algorithms (rows 2–7, 14, 15) are **computed by hand** and
  emitted as `GL_POINTS` — no `GL_LINES`/`GL_POLYGON` shortcuts for those.
* The 3D visibility/transform rows (16–22) deliberately use OpenGL's built-ins;
  the **software Z-buffer** in `softzbuffer.cpp` additionally proves the depth
  algorithm itself (two overlapping triangles resolved with our own depth array).
* The projection morph (rows 19–20) lerps a hand-built **orthographic** matrix
  into a hand-built **perspective** matrix (`glLoadMatrixf`), so the "flat skyline
  gains a vanishing point" is a single continuous keyframed transition.
* Everything is driven by the **keyframe timeline** (row 23): one `t` clock feeds
  `Track::sample`, which interpolates a handful of keyframes per animated value.
