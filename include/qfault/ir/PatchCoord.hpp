#pragma once

namespace qfault {

struct PatchCoord {
    int x{0};
    int y{0};

    [[nodiscard]] bool operator==(const PatchCoord&) const noexcept = default;

    // Required for use as std::map key in later routing passes.
    [[nodiscard]] bool operator<(const PatchCoord& rhs) const noexcept {
        if (x != rhs.x) return x < rhs.x;
        return y < rhs.y;
    }
};

} // namespace qfault
