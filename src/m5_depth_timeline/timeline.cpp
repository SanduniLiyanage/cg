// m5_depth_timeline/timeline.cpp — the master keyframe timeline.
// Every animated value in the film is a handful of keyframes, interpolated.
#include "m5_depth_timeline/timeline.h"
#include <cmath>

namespace hw { namespace m5 {

static Color C(float r,float g,float b,float a=1){ return {r,g,b,a}; }
static Vec2  V(float x,float y){ return {x,y}; }

void updateTimeline(SceneState& s, float t) {
    s.t = t;

    // --- Segment index -----------------------------------------------------
    s.segment = 0;
    for (int i = 0; i < 6; ++i) if (t >= SEG_TIMES[i]) s.segment = i;

    // --- Sky gradient (pre-dawn -> dawn -> day -> sunset -> night) ----------
    static Track<Color> skyTop = {
        {0,   C(0.04f,0.04f,0.13f)}, {25, C(0.16f,0.18f,0.42f)},
        {45,  C(0.30f,0.46f,0.78f)}, {80, C(0.20f,0.52f,0.95f)},
        {115, C(0.32f,0.24f,0.52f)}, {138,C(0.55f,0.26f,0.40f)},
        {150, C(0.05f,0.06f,0.20f)}, {200,C(0.02f,0.03f,0.10f)},
        {230, C(0.02f,0.03f,0.10f)},
    };
    static Track<Color> skyBot = {
        {0,   C(0.10f,0.08f,0.20f)}, {25, C(0.45f,0.32f,0.42f)},
        {45,  C(0.99f,0.66f,0.45f)}, {80, C(0.70f,0.88f,1.00f)},
        {115, C(1.00f,0.55f,0.28f)}, {138,C(1.00f,0.40f,0.22f)},
        {150, C(0.18f,0.12f,0.30f)}, {200,C(0.08f,0.08f,0.18f)},
        {230, C(0.08f,0.08f,0.18f)},
    };
    s.skyTop = skyTop.sample(t);
    s.skyBottom = skyBot.sample(t);

    // --- Stars & moon ------------------------------------------------------
    static Track<float> starA = {
        {0,1.0f},{22,0.9f},{32,0.0f},{140,0.0f},{158,1.0f},{230,1.0f}
    };
    s.starAlpha = starA.sample(t);
    static Track<float> moonA = {
        {0,0.0f},{140,0.0f},{160,0.9f},{230,0.9f}
    };
    s.moonAlpha = moonA.sample(t);
    s.moonPos = V(940, 600);
    s.moonR = 34;

    // --- Sun: midpoint circle on a keyframed arc (screen-relative) ----------
    static Track<Vec2> sunP = {
        {0,  V(150, 150)}, {25, V(220, 250)}, {45, V(360, 470)},
        {85, V(760, 610)}, {120,V(1040,430)}, {145,V(1180,200)},
        {150,V(1220,120)},
    };
    s.sunPos = sunP.sample(t);
    static Track<float> sunRk = {{0,40},{85,48},{145,58},{150,58}};
    s.sunR = sunRk.sample(t);
    static Track<Color> sunC = {
        {0,  C(1.0f,0.80f,0.55f)}, {45, C(1.0f,0.86f,0.55f)},
        {85, C(1.0f,0.95f,0.75f)}, {120,C(1.0f,0.62f,0.30f)},
        {145,C(1.0f,0.40f,0.22f)},
    };
    s.sunCore = sunC.sample(t);
    s.sunGlow = s.sunCore; s.sunGlow.a = 0.22f;

    // --- Camera scroll over the wide 2D world (drives parallax + window) ----
    static Track<float> cam = {
        {0,0.0f},{25,0.0f},{60,460.0f},{105,1050.0f},{145,1500.0f},{150,1500.0f}
    };
    s.camX = cam.sample(t);
    s.windAngle = std::sin(t * 0.7f) * 0.12f;

    // --- Balloon: inflation (scale), drift (translate), sway (rotate) -------
    static Track<float> infl = {{0,0.04f},{5,0.06f},{22,1.0f},{230,1.0f}};
    s.balloonInflate = infl.sample(t);
    static Track<Vec2> balP = {
        {0,  V(300, 250)}, {22, V(300, 300)}, {60, V(560, 470)},
        {105,V(720, 510)}, {145,V(840, 520)},
    };
    s.balloonPos = balP.sample(t);
    s.balloonSway = (t > 22.0f) ? std::sin(t * 0.8f) * 0.05f : 0.0f;
    s.flameFlicker = 0.8f + 0.2f * std::sin(t * 11.0f);

    // --- Golden-hour shadow shear ------------------------------------------
    static Track<float> shear = {
        {0,0.3f},{60,0.25f},{90,0.5f},{120,1.9f},{145,1.4f},{150,1.4f}
    };
    s.shadowShear = shear.sample(t);

    // --- Rain shower (Liang-Barsky) in golden hour -------------------------
    static Track<float> rain = {
        {0,0},{108,0},{116,1.0f},{132,1.0f},{140,0},{230,0}
    };
    s.rainAmount = rain.sample(t);

    // --- Shooting star: a quick streak late in golden hour ------------------
    if (t >= 135.0f && t <= 139.0f) s.shootingStar = (t - 135.0f) / 4.0f;
    else s.shootingStar = -1.0f;

    s.reflectAlpha = (t < 130) ? 0.55f : 0.40f;

    // --- City (segment 4): parallel skyline -> perspective descent ---------
    static Track<float> reveal = {{0,0},{145,0.0f},{150,0.05f},{176,1.0f},{230,1.0f}};
    s.cityReveal = reveal.sample(t);
    static Track<float> rise = {{0,0},{145,0.0f},{172,1.0f},{230,1.0f}};
    s.cityRise = rise.sample(t);
    static Track<float> cscale = {{0,0.55f},{145,0.55f},{178,1.0f},{230,1.0f}};
    s.cityScale = cscale.sample(t);
    static Track<float> orbit = {{0,0},{145,-38.0f},{195,26.0f},{230,30.0f}};
    s.camOrbit = orbit.sample(t);
    static Track<float> camH = {{0,0},{145,1.0f},{198,0.12f},{215,0.08f},{230,0.08f}};
    s.camHeight = camH.sample(t);

    // Veil that masks the 2D->3D hand-off at t=CITY_T (a short tent pulse).
    float d = std::fabs(t - CITY_T);
    s.city2DFade = (d < 4.0f) ? (1.0f - d / 4.0f) : 0.0f;

    // --- Active concept captions for the current moment --------------------
    s.activeConcepts.clear();
    switch (s.segment) {
        case 0: s.activeConcepts = {"Points (stars)","DDA line (tether ropes)",
                  "Midpoint circle (envelope)","2D scaling (inflation)"}; break;
        case 1: s.activeConcepts = {"Flood fill (sky & lake)","Boundary fill (sun disc)",
                  "Midpoint circle (sun)","Scan-line fill (mountains)",
                  "Translation (parallax)","Reflection (lake)",
                  "Keyframe interpolation (sun arc)"}; break;
        case 2: s.activeConcepts = {"Window->viewport mapping","Cohen-Sutherland (birds)",
                  "Rotation (windmill & sway)","Shearing (flags)",
                  "Bresenham line (power lines)","Scan-line fill (flags)"}; break;
        case 3: s.activeConcepts = {"Shearing (long shadows)","Liang-Barsky (slanted rain)",
                  "Liang-Barsky (shooting star)"}; break;
        case 4: s.activeConcepts = {"Parallel projection (flat skyline)",
                  "Perspective projection (vanishing point)","3D rotation (camera orbit)",
                  "3D translation (city rises)","3D scaling (grow into view)",
                  "Hidden surface removal (back-face cull)","Z-buffer (depth test)"}; break;
        case 5: s.activeConcepts = {"Midpoint circle (lanterns & moon)",
                  "Keyframes & interpolation (whole journey)"}; break;
    }
}

} } // namespace hw::m5
