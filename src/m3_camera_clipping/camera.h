// m3_camera_clipping — The Camera (framing & clipping).
// Window->viewport mapping (set up in main) plus Cohen-Sutherland (birds) and
// Liang-Barsky (slanted rain & shooting star) clipping at the frame border.
#pragma once
#include "common/scene.h"

namespace hw { namespace m3 {

// Map a point from the world WINDOW [wl,wr]x[wb,wt] to the screen VIEWPORT.
Vec2 windowToViewport(Vec2 p, float wl, float wr, float wb, float wt,
                      float vx, float vy, float vw, float vh);

// Draw birds, rain and the shooting star — each clipped to the visible frame.
void drawClipped(const SceneState& s);

} } // namespace hw::m3
