// m2_fills_transforms/transforms.cpp — hand-coded 2D affine transforms.
#include "m2_fills_transforms/transforms.h"
#include <cmath>

namespace hw { namespace m2 {

Vec2 translateP(Vec2 p, float dx, float dy) { return { p.x + dx, p.y + dy }; }

Vec2 scaleP(Vec2 p, Vec2 c, float sx, float sy) {
    return { c.x + (p.x - c.x) * sx, c.y + (p.y - c.y) * sy };
}

Vec2 rotateP(Vec2 p, Vec2 c, float ang) {
    float s = std::sin(ang), co = std::cos(ang);
    float dx = p.x - c.x, dy = p.y - c.y;
    return { c.x + dx * co - dy * s, c.y + dx * s + dy * co };
}

// Reflection across a horizontal line y = axisY  (lake surface).
Vec2 reflectY(Vec2 p, float axisY) { return { p.x, 2.0f * axisY - p.y }; }

// Shear in X proportional to height above baseY  (long leaning shadows).
Vec2 shearX(Vec2 p, float baseY, float k) { return { p.x + k * (p.y - baseY), p.y }; }

std::vector<Vec2> translate(const std::vector<Vec2>& v, float dx, float dy) {
    std::vector<Vec2> o; o.reserve(v.size());
    for (auto p : v) o.push_back(translateP(p, dx, dy));
    return o;
}
std::vector<Vec2> scale(const std::vector<Vec2>& v, Vec2 c, float sx, float sy) {
    std::vector<Vec2> o; o.reserve(v.size());
    for (auto p : v) o.push_back(scaleP(p, c, sx, sy));
    return o;
}
std::vector<Vec2> rotate(const std::vector<Vec2>& v, Vec2 c, float ang) {
    std::vector<Vec2> o; o.reserve(v.size());
    for (auto p : v) o.push_back(rotateP(p, c, ang));
    return o;
}
std::vector<Vec2> reflectY(const std::vector<Vec2>& v, float axisY) {
    std::vector<Vec2> o; o.reserve(v.size());
    for (auto p : v) o.push_back(reflectY(p, axisY));
    return o;
}
std::vector<Vec2> shearX(const std::vector<Vec2>& v, float baseY, float k) {
    std::vector<Vec2> o; o.reserve(v.size());
    for (auto p : v) o.push_back(shearX(p, baseY, k));
    return o;
}

} } // namespace hw::m2
