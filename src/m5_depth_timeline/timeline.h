// m5_depth_timeline — The Director (part 1: keyframes & interpolation).
// The master timeline: a handful of keyframes per track, smoothly interpolated
// every frame to drive sky colour, sun, balloon, camera and the city descent.
#pragma once
#include "common/types.h"
#include "common/scene.h"
#include <vector>
#include <algorithm>

namespace hw { namespace m5 {

// One keyframe = a time (seconds) and a value, with optional eased blend in.
template <typename T>
struct Key { float t; T v; bool ease = true; };

template <typename T>
struct Track {
    std::vector<Key<T>> keys;
    Track(std::initializer_list<Key<T>> k) : keys(k) {}
    T sample(float t) const {
        if (keys.empty()) return T{};
        if (t <= keys.front().t) return keys.front().v;
        if (t >= keys.back().t)  return keys.back().v;
        for (size_t i = 0; i + 1 < keys.size(); ++i) {
            const auto& a = keys[i];
            const auto& b = keys[i + 1];
            if (t >= a.t && t <= b.t) {
                float u = (t - a.t) / (b.t - a.t);
                if (b.ease) u = smooth(u);
                return lerp(a.v, b.v, u);
            }
        }
        return keys.back().v;
    }
};

// Fill g_scene from the timeline at time t. Called once per frame.
void updateTimeline(SceneState& s, float t);

} } // namespace hw::m5
