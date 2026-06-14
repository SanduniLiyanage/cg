// m2_fills_transforms — The Painter (part 3: the 2D world).
// Composes the day-time scene (segments 0-3) from M1 primitives + M2 fills and
// transforms: sky, parallax landscape, lake reflection, and the balloon with
// inflation (scale), sway (rotate), shadow (shear).
#pragma once
#include "common/scene.h"

namespace hw { namespace m2 {

void initWorld2D();                  // builds cached flood-fill regions once
void drawWorld2D(const SceneState& s);

// The balloon is reused by M3 (drift) and reflected in the lake; exposed so the
// reflection pass can redraw it mirrored.
void drawBalloon(const SceneState& s, bool reflected);

} } // namespace hw::m2
