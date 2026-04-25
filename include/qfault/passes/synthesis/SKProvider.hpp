#pragma once

#include <qfault/ir/GateKind.hpp>
#include <qfault/passes/synthesis/SynthesisProvider.hpp>

#include <string_view>
#include <vector>

namespace qfault {

// Solovay-Kitaev synthesis provider (pure C++, no external dependencies).
// Depth-3 recursion; guaranteed operator-norm error ≤ 1e-3 for any R_z(angle).
// Use as a benchmark baseline only — GridSynth produces shorter T-counts (ADR-0004).
class SKProvider {
public:
    [[nodiscard]] std::vector<GateKind> synthesise(double angle, double eps);
    [[nodiscard]] std::string_view name() const noexcept { return "SKProvider"; }
};

static_assert(SynthesisProvider<SKProvider>);

} // namespace qfault
