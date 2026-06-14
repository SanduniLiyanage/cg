// m2_fills_transforms — The Painter (part 1: fills).
// Hand-written area fills: scan-line (polygons), and seed fills (boundary &
// flood) computed on a small logical Canvas grid, then plotted as GL_POINTS.
#pragma once
#include "common/types.h"
#include <vector>

namespace hw { namespace m2 {

// --- Scan-line polygon fill -----------------------------------------------
// Active-edge intersection per scan row; fills between sorted x crossings.
// Used for mountains, hills, rooftops, pennant flags, basket.
void scanlineFill(const std::vector<Vec2>& poly, const Color& c);

// --- Seed-fill Canvas ------------------------------------------------------
// A tiny logical pixel grid (offset into world space). We rasterise a boundary
// into it, run the seed fill, then draw the filled cells. This is the honest
// boundary/flood-fill demonstration without scanning the whole framebuffer.
struct Canvas {
    int ox, oy, w, h;
    std::vector<unsigned char> m;     // 0 empty, 1 boundary, 2 filled
    Canvas(int ox_, int oy_, int w_, int h_)
        : ox(ox_), oy(oy_), w(w_), h(h_), m(w_ * h_, 0) {}
    bool inside(int x, int y) const { return x>=0 && y>=0 && x<w && y<h; }
    unsigned char& at(int x, int y) { return m[y*w + x]; }
    unsigned char  get(int x, int y) const { return m[y*w + x]; }
};

// Mark a midpoint-circle outline into a canvas as boundary (value 1).
void markCircleBoundary(Canvas& cv, int cx, int cy, int r);
// Mark a polygon outline (DDA edges) into a canvas as boundary.
void markPolyBoundary(Canvas& cv, const std::vector<Vec2>& poly);

// Boundary fill (4-connected): fill cells until the boundary value is hit.
void boundaryFill4(Canvas& cv, int sx, int sy);
// Flood fill (4-connected): replace the target region (value 0) with fill.
void floodFill4(Canvas& cv, int sx, int sy);

// Plot every filled cell of a canvas at world coordinates with colour c.
void drawFilled(const Canvas& cv, const Color& c);

} } // namespace hw::m2
