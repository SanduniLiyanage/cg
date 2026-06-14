// common/types.h — shared math types and helpers used by every module.
// Kept tiny and header-only so all five member systems can share them.
#pragma once
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace hw {

constexpr float PI = 3.14159265358979323846f;

struct Vec2 { float x = 0, y = 0; };
struct Vec3 { float x = 0, y = 0, z = 0; };
struct Color { float r = 0, g = 0, b = 0, a = 1; };

inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// Linear interpolation overloads — the heart of the keyframe system (M5).
inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
inline Vec2  lerp(const Vec2& a, const Vec2& b, float t) {
    return { lerp(a.x, b.x, t), lerp(a.y, b.y, t) };
}
inline Color lerp(const Color& a, const Color& b, float t) {
    return { lerp(a.r, b.r, t), lerp(a.g, b.g, t),
             lerp(a.b, b.b, t), lerp(a.a, b.a, t) };
}

// Smoothstep easing so keyframes ramp in/out instead of moving linearly.
inline float smooth(float t) {
    t = clampf(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

inline float deg2rad(float d) { return d * PI / 180.0f; }

} // namespace hw
