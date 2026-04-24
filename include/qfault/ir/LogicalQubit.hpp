#pragma once

#include <cstddef>
#include <string>

namespace qfault {

struct LogicalQubit {
    std::string name;
    std::size_t index{0};

    [[nodiscard]] bool operator==(const LogicalQubit&) const noexcept = default;
};

} // namespace qfault
