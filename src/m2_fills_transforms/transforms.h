// m2_fills_transforms — The Painter (part 2: 2D affine transforms).
// All hand-applied to point lists so the maths is visible (no GL matrix stack).
#pragma once
#include "common/types.h"
#include <vector>

namespace hw { namespace m2 {

Vec2 translateP(Vec2 p, float dx, float dy);
Vec2 scaleP    (Vec2 p, Vec2 c, float sx, float sy);   // about pivot c
Vec2 rotateP   (Vec2 p, Vec2 c, float ang);            // radians, about c
Vec2 reflectY  (Vec2 p, float axisY);                  // mirror across y=axisY
Vec2 shearX    (Vec2 p, float baseY, float k);         // x += k*(y-baseY)

// Whole-polygon convenience wrappers.
std::vector<Vec2> translate(const std::vector<Vec2>&, float dx, float dy);
std::vector<Vec2> scale    (const std::vector<Vec2>&, Vec2 c, float sx, float sy);
std::vector<Vec2> rotate   (const std::vector<Vec2>&, Vec2 c, float ang);
std::vector<Vec2> reflectY (const std::vector<Vec2>&, float axisY);
std::vector<Vec2> shearX   (const std::vector<Vec2>&, float baseY, float k);

} } // namespace hw::m2
