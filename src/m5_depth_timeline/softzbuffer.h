// m5_depth_timeline — The Director (part 2: depth & visibility).
// The 3D balloon that weaves between the towers (resolved by the hardware
// Z-buffer / depth test) and a tiny *software* Z-buffer proof for top marks.
#pragma once
#include "common/scene.h"

namespace hw { namespace m5 {

// The balloon rendered as a 3D object so the depth buffer decides, per pixel,
// whether each tower is in front of or behind it.
void drawBalloon3D(const SceneState& s);

// Standalone software Z-buffer: two overlapping triangles resolved with our own
// depth array, drawn in a HUD corner (shown when labels are on).
void drawSoftZBufferDemo();

} } // namespace hw::m5
