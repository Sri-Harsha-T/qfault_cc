#pragma once

#include <qfault/ir/GateKind.hpp>
#include <qfault/passes/PassContext.hpp>
#include <qfault/passes/synthesis/SynthesisProvider.hpp>

#include <string_view>
#include <vector>

namespace qfault {

// Wraps the external GridSynth binary as a SynthesisProvider.
// If the binary is unavailable (GRIDSYNTH_BINARY not set at build time or
// the path does not exist at runtime), synthesise() returns an empty vector
// and logs DiagLevel::Warn via the supplied PassContext.
//
// Per ADR-0004, GridSynth is the default provider. Do not substitute SKProvider
// unless benchmarking.
class GridSynthProvider {
public:
    // ctx is used solely for diagnostic logging when the binary is absent.
    explicit GridSynthProvider(PassContext& ctx) noexcept : ctx_{ctx} {}

    [[nodiscard]] std::vector<GateKind> synthesise(double angle, double eps);
    [[nodiscard]] std::string_view name() const noexcept { return "GridSynthProvider"; }

private:
    PassContext& ctx_;
};

static_assert(SynthesisProvider<GridSynthProvider>);

} // namespace qfault
