// m2_fills_transforms/fills.cpp — scan-line, boundary and flood fill.
#include "m2_fills_transforms/fills.h"
#include "m1_primitives/primitives.h"
#include <GL/freeglut.h>
#include <algorithm>
#include <cmath>

namespace hw { namespace m2 {

// ---------------------------------------------------------------------------
// Scan-line polygon fill.
void scanlineFill(const std::vector<Vec2>& poly, const Color& c) {
    int n = (int)poly.size();
    if (n < 3) return;
    float ymin = poly[0].y, ymax = poly[0].y;
    for (const auto& p : poly) { ymin = std::min(ymin, p.y); ymax = std::max(ymax, p.y); }
    glColor4f(c.r, c.g, c.b, c.a);
    int y0 = (int)std::ceil(ymin), y1 = (int)std::floor(ymax);
    std::vector<float> xs;
    for (int y = y0; y <= y1; ++y) {
        xs.clear();
        float fy = (float)y;
        for (int i = 0; i < n; ++i) {
            const Vec2& a = poly[i];
            const Vec2& b = poly[(i + 1) % n];
            // Half-open rule [min,max) avoids double-counting shared vertices.
            if ((a.y <= fy && b.y > fy) || (b.y <= fy && a.y > fy)) {
                float x = a.x + (fy - a.y) / (b.y - a.y) * (b.x - a.x);
                xs.push_back(x);
            }
        }
        std::sort(xs.begin(), xs.end());
        glBegin(GL_POINTS);
        for (size_t k = 0; k + 1 < xs.size(); k += 2) {
            int xa = (int)std::ceil(xs[k]);
            int xb = (int)std::floor(xs[k + 1]);
            for (int x = xa; x <= xb; ++x) glVertex2i(x, y);
        }
        glEnd();
    }
}

// ---------------------------------------------------------------------------
// Canvas boundary rasterisation.
void markCircleBoundary(Canvas& cv, int cx, int cy, int r) {
    // Mark a ~2px-thick ring so the 4-connected seed fill can never leak
    // through a diagonal gap (a 1px midpoint ring is not 4-connected).
    for (int y = 0; y < cv.h; ++y)
        for (int x = 0; x < cv.w; ++x) {
            float dx = (float)(x + cv.ox - cx), dy = (float)(y + cv.oy - cy);
            float d = std::sqrt(dx*dx + dy*dy);
            if (d >= r - 1.0f && d <= r + 0.3f) cv.at(x, y) = 1;
        }
}

void markPolyBoundary(Canvas& cv, const std::vector<Vec2>& poly) {
    int n = (int)poly.size();
    for (int i = 0; i < n; ++i) {
        int x0 = (int)poly[i].x - cv.ox,       y0 = (int)poly[i].y - cv.oy;
        int x1 = (int)poly[(i+1)%n].x - cv.ox, y1 = (int)poly[(i+1)%n].y - cv.oy;
        // integer DDA into the grid
        int dx = std::abs(x1-x0), dy = std::abs(y1-y0);
        int steps = std::max(dx, dy); if (steps == 0) steps = 1;
        float xi = (x1-x0)/(float)steps, yi = (y1-y0)/(float)steps;
        float x = x0, y = y0;
        for (int s = 0; s <= steps; ++s) {
            int ix=(int)std::lround(x), iy=(int)std::lround(y);
            if (cv.inside(ix, iy)) cv.at(ix, iy) = 1;
            x += xi; y += yi;
        }
    }
}

// Iterative 4-connected seed fills (stack-based to avoid deep recursion).
void boundaryFill4(Canvas& cv, int sx, int sy) {
    if (!cv.inside(sx, sy) || cv.get(sx, sy) != 0) return;
    std::vector<std::pair<int,int>> st;
    st.push_back({sx, sy});
    while (!st.empty()) {
        auto [x, y] = st.back(); st.pop_back();
        if (!cv.inside(x, y)) continue;
        unsigned char& v = cv.at(x, y);
        if (v != 0) continue;          // boundary(1) or already filled(2)
        v = 2;
        st.push_back({x+1, y}); st.push_back({x-1, y});
        st.push_back({x, y+1}); st.push_back({x, y-1});
    }
}

void floodFill4(Canvas& cv, int sx, int sy) {
    // Same traversal; semantically replaces the "target" (empty=0) region.
    boundaryFill4(cv, sx, sy);
}

void drawFilled(const Canvas& cv, const Color& c) {
    glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_POINTS);
    for (int y = 0; y < cv.h; ++y)
        for (int x = 0; x < cv.w; ++x)
            if (cv.get(x, y) == 2) glVertex2i(x + cv.ox, y + cv.oy);
    glEnd();
}

} } // namespace hw::m2
