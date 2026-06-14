// m1_primitives.cpp — hand-coded DDA, Bresenham, midpoint circle.
#include "m1_primitives/primitives.h"
#include <GL/freeglut.h>
#include <cmath>

namespace hw { namespace m1 {

void setColor(const Color& c) { glColor4f(c.r, c.g, c.b, c.a); }
void setColor(float r, float g, float b, float a) { glColor4f(r, g, b, a); }

// Each plotted pixel is an explicit GL_POINTS vertex — this IS the demo.
void putPixel(int x, int y) {
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

// Batch a run of pixels without re-opening glBegin for every point.
static inline void plot(int x, int y) { glVertex2i(x, y); }

// ---------------------------------------------------------------------------
// DDA line — Digital Differential Analyzer.
// Step along the longer axis, advance the other by a fractional increment.
void ddaLine(int x0, int y0, int x1, int y1) {
    int dx = x1 - x0, dy = y1 - y0;
    int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) { putPixel(x0, y0); return; }
    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;
    float x = (float)x0, y = (float)y0;
    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; ++i) {
        plot((int)std::lround(x), (int)std::lround(y));
        x += xInc; y += yInc;
    }
    glEnd();
}

// ---------------------------------------------------------------------------
// Bresenham line — pure integer arithmetic, no rounding per step.
void bresenhamLine(int x0, int y0, int x1, int y1) {
    int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    glBegin(GL_POINTS);
    while (true) {
        plot(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
    glEnd();
}

// ---------------------------------------------------------------------------
// Midpoint circle — decision parameter, 8-way symmetric plotting.
static inline void plot8(int cx, int cy, int x, int y) {
    plot(cx + x, cy + y); plot(cx - x, cy + y);
    plot(cx + x, cy - y); plot(cx - x, cy - y);
    plot(cx + y, cy + x); plot(cx - y, cy + x);
    plot(cx + y, cy - x); plot(cx - y, cy - x);
}

void midpointCircle(int cx, int cy, int r) {
    if (r <= 0) { putPixel(cx, cy); return; }
    int x = 0, y = r;
    int p = 1 - r;
    glBegin(GL_POINTS);
    plot8(cx, cy, x, y);
    while (x < y) {
        ++x;
        if (p < 0) { p += 2 * x + 1; }
        else { --y; p += 2 * (x - y) + 1; }
        plot8(cx, cy, x, y);
    }
    glEnd();
}

std::vector<Vec2> circlePoints(int cx, int cy, int r) {
    std::vector<Vec2> out;
    if (r <= 0) { out.push_back({(float)cx,(float)cy}); return out; }
    int x = 0, y = r, p = 1 - r;
    auto push8 = [&](int X, int Y){
        out.push_back({(float)(cx+X),(float)(cy+Y)});
        out.push_back({(float)(cx-X),(float)(cy+Y)});
        out.push_back({(float)(cx+X),(float)(cy-Y)});
        out.push_back({(float)(cx-X),(float)(cy-Y)});
        out.push_back({(float)(cx+Y),(float)(cy+X)});
        out.push_back({(float)(cx-Y),(float)(cy+X)});
        out.push_back({(float)(cx+Y),(float)(cy-X)});
        out.push_back({(float)(cx-Y),(float)(cy-X)});
    };
    push8(x, y);
    while (x < y) {
        ++x;
        if (p < 0) p += 2 * x + 1;
        else { --y; p += 2 * (x - y) + 1; }
        push8(x, y);
    }
    return out;
}

// A star: a bright centre pixel plus a faint plus-shaped twinkle.
void pointStar(int x, int y, float size) {
    glBegin(GL_POINTS);
    plot(x, y);
    if (size > 1.2f) { plot(x+1,y); plot(x-1,y); plot(x,y+1); plot(x,y-1); }
    glEnd();
}

void polylineDDA(const std::vector<Vec2>& pts, bool closed) {
    if (pts.size() < 2) return;
    for (size_t i = 0; i + 1 < pts.size(); ++i)
        ddaLine((int)pts[i].x,(int)pts[i].y,(int)pts[i+1].x,(int)pts[i+1].y);
    if (closed)
        ddaLine((int)pts.back().x,(int)pts.back().y,(int)pts.front().x,(int)pts.front().y);
}

} } // namespace hw::m1
