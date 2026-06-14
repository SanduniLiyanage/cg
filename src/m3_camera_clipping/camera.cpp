// m3_camera_clipping/camera.cpp — STUB (filled in at the M3 build step).
#include "m3_camera_clipping/camera.h"

namespace hw { namespace m3 {

Vec2 windowToViewport(Vec2 p, float wl, float wr, float wb, float wt,
                      float vx, float vy, float vw, float vh) {
    float sx = (p.x - wl) / (wr - wl) * vw + vx;
    float sy = (p.y - wb) / (wt - wb) * vh + vy;
    return { sx, sy };
}

void drawClipped(const SceneState&) {}

} } // namespace hw::m3
