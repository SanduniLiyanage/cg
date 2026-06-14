// m1_primitives — The Draftsman.
// Hand-written rasterisation: every line is computed pixel-by-pixel here and
// plotted with glBegin(GL_POINTS); glVertex2i; glEnd();  — never GL_LINES.
#pragma once
#include "common/types.h"
#include <vector>

namespace hw { namespace m1 {

// Set the colour used by the plotting helpers below.
void setColor(const Color& c);
void setColor(float r, float g, float b, float a = 1.0f);

// Plot a single pixel (the only primitive OpenGL actually draws for us in 2D).
void putPixel(int x, int y);

// --- Lines ---------------------------------------------------------------
// DDA: floating-point stepping. Used for the balloon's diagonal tether ropes.
void ddaLine(int x0, int y0, int x1, int y1);
// Bresenham: integer decision variable. Used for power lines & mountain ridges.
void bresenhamLine(int x0, int y0, int x1, int y1);

// --- Circle --------------------------------------------------------------
// Midpoint circle (8-way symmetry). Sun, moon, balloon envelope, lanterns.
void midpointCircle(int cx, int cy, int r);
// Convenience: midpoint circle outline as a vector of boundary points
// (used by M2's boundary fill to know where to stop).
std::vector<Vec2> circlePoints(int cx, int cy, int r);

// --- Points / polygons ---------------------------------------------------
void pointStar(int x, int y, float size); // a star = a small cluster of points
void polylineDDA(const std::vector<Vec2>& pts, bool closed);

} } // namespace hw::m1
