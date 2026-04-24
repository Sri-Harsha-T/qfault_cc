#pragma once

#include <qfault/ir/MeasBasis.hpp>
#include <qfault/ir/PatchCoord.hpp>
#include <qfault/ir/PatchOpKind.hpp>

#include <cstddef>
#include <vector>

namespace qfault {

struct PatchOp {
    PatchOpKind kind{PatchOpKind::IDLE};
    std::vector<PatchCoord> patches;
    MeasBasis basis{MeasBasis::Z};
    std::size_t timeStep{0}; // negative time steps are a logic error — use size_t

    [[nodiscard]] bool operator==(const PatchOp&) const noexcept = default;
};

} // namespace qfault
