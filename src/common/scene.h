// common/scene.h — the single shared scene state every system reads & writes.
// M5's timeline fills these fields each frame; M1..M4 render from them.
#pragma once
#include "common/types.h"
#include <string>
#include <vector>

namespace hw {

// ---- Window / world layout ------------------------------------------------
constexpr int   WIN_W = 1280;
constexpr int   WIN_H = 720;
// The 2D world is wider than the screen so the camera (M3) is a moving
// *window* over it. World height matches the screen height (1 unit = 1 pixel).
constexpr float WORLD_H = (float)WIN_H;
constexpr float GROUND_Y = 235.0f;        // horizon / ground line in world Y
constexpr float WATER_TOP = GROUND_Y;     // lake surface line (reflection axis)

// ---- Timeline (seconds) ---------------------------------------------------
// Six segments, played start->finish then looped. Numbers match the script.
constexpr float SEG_TIMES[7] = {
    0.0f,    // Seg 0 start  — pre-dawn inflation
    25.0f,   // Seg 1        — dawn liftoff
    60.0f,   // Seg 2        — daytime drift
    105.0f,  // Seg 3        — golden hour
    145.0f,  // Seg 4        — descent into the city (2D -> 3D)
    200.0f,  // Seg 5        — touchdown
    230.0f   // end / loop
};
constexpr float CYCLE = 230.0f;

// The 2D world plays in segments 0-3; the 3D city owns 4-5. CITY_T is when the
// renderer hands over from gluOrtho2D to the 3D projection.
constexpr float CITY_T = SEG_TIMES[4];

// ---- Shared, per-frame scene state ---------------------------------------
struct SceneState {
    float t = 0.0f;          // seconds into the current cycle
    int   segment = 0;       // 0..5, derived from t
    bool  labelsOn = false;  // 'L' toggle — concept captions off by default
    bool  paused = false;

    // Sky gradient (M2 flood-fill / gradient quad uses these).
    Color skyTop, skyBottom;
    float starAlpha = 1.0f;  // star visibility (night/pre-dawn)

    // Sun & moon (M1 midpoint circles, M2 boundary fill).
    Vec2  sunPos;  float sunR = 46.0f;  Color sunCore, sunGlow;
    Vec2  moonPos; float moonR = 34.0f; float moonAlpha = 0.0f;

    // Camera window over the wide 2D world (M3 window->viewport mapping).
    float camX = 0.0f;       // left edge of the camera window in world X
    float windAngle = 0.0f;  // global "wind" used by flags/windmill/rain

    // Balloon (M1 circle, M2 transforms: scale=inflate, rotate=sway).
    Vec2  balloonPos;        // world position of the envelope centre
    float balloonInflate = 0.0f; // 0 = crumpled on ground, 1 = full canopy
    float balloonSway = 0.0f;    // radians, gentle rotation
    float flameFlicker = 1.0f;

    // Atmosphere driven by keyframes.
    float shadowShear = 0.0f; // M2 shear factor for long shadows
    float rainAmount  = 0.0f; // M3 Liang-Barsky rain density 0..1
    float shootingStar = -1.0f; // 0..1 progress, <0 = inactive
    float reflectAlpha = 0.55f; // lake reflection strength

    // 3D city (M4 projection / transforms, M5 depth).
    float cityReveal = 0.0f;  // 0 = flat far skyline, 1 = full perspective city
    float cityRise   = 0.0f;  // 3D translation of the city into view
    float cityScale  = 1.0f;  // 3D scaling (grow into view)
    float camOrbit   = 0.0f;  // degrees, 3D camera rotation around the city
    float camHeight  = 1.0f;  // descent altitude for gluLookAt
    float city2DFade = 0.0f;  // veil that hides the 2D->3D hand-off

    // Active concept captions for the current moment (shown when labelsOn).
    std::vector<std::string> activeConcepts;
};

extern SceneState g_scene;

} // namespace hw
