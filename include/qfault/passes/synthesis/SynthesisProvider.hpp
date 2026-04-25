#pragma once

#include <qfault/ir/GateKind.hpp>

#include <concepts>
#include <string_view>
#include <vector>

namespace qfault {

// A type satisfies SynthesisProvider if it can approximate R_z(angle) to within
// epsilon (operator-norm error) and report a human-readable name.
//
// Angle is in radians. Returns a Clifford+T gate sequence that implements the
// approximation; empty vector signals "unavailable" (e.g. binary not found).
template <typename T>
concept SynthesisProvider = requires(T p, double angle, double eps) {
    { p.synthesise(angle, eps) } -> std::convertible_to<std::vector<GateKind>>;
    { p.name() }                 -> std::convertible_to<std::string_view>;
};

} // namespace qfault
