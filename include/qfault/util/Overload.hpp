#pragma once

namespace qfault {

// Overload helper for std::visit with multiple lambdas.
// Usage: std::visit(overload{[](LogicalGate&){...}, [](PatchOp&){...}}, instr);
template <class... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

} // namespace qfault
