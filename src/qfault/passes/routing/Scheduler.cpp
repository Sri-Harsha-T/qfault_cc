#include <qfault/passes/routing/Scheduler.hpp>

#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/PauliFrameUpdate.hpp>

#include <algorithm>
#include <map>
#include <variant>
#include <vector>

namespace qfault {

std::vector<std::vector<std::size_t>>
EAFScheduler::buildDepGraph(const std::vector<std::size_t>& gate_positions,
                             const std::vector<Instruction>& instructions) {
    const std::size_t n = gate_positions.size();
    std::vector<std::vector<std::size_t>> preds(n);

    // Maps qubit index → position of the last gate that touched it.
    std::map<std::size_t, std::size_t> last_writer;

    for (std::size_t pos = 0; pos < n; ++pos) {
        const auto& gate = std::get<LogicalGate>(instructions[gate_positions[pos]]);
        for (const auto& q : gate.operands) {
            const auto it = last_writer.find(q.index);
            if (it != last_writer.end()) {
                preds[pos].push_back(it->second);
            }
            last_writer[q.index] = pos;
        }
    }
    return preds;
}

std::vector<ScheduleSlot>
EAFScheduler::schedule(const std::vector<Instruction>& instructions) {
    // Collect positions of LogicalGate instructions only.
    std::vector<std::size_t> gate_positions;
    gate_positions.reserve(instructions.size());
    for (std::size_t i = 0; i < instructions.size(); ++i) {
        if (std::holds_alternative<LogicalGate>(instructions[i])) {
            gate_positions.push_back(i);
        }
    }

    if (gate_positions.empty()) {
        depth_ = 0;
        return {};
    }

    const std::size_t n = gate_positions.size();
    auto preds = buildDepGraph(gate_positions, instructions);

    // Build successor lists and compute in-degrees; deduplicate preds first.
    std::vector<std::vector<std::size_t>> succs(n);
    std::vector<std::size_t> in_deg(n, 0);

    for (std::size_t pos = 0; pos < n; ++pos) {
        auto& p = preds[pos];
        std::sort(p.begin(), p.end());
        p.erase(std::unique(p.begin(), p.end()), p.end());
        in_deg[pos] = p.size();
        for (const std::size_t pred : p) {
            succs[pred].push_back(pos);
        }
    }

    // Kahn's topological-layer algorithm:
    // each "frontier" of in-degree-zero nodes forms one ScheduleSlot.
    std::vector<ScheduleSlot> slots;
    std::vector<std::size_t> frontier;
    frontier.reserve(n);
    for (std::size_t pos = 0; pos < n; ++pos) {
        if (in_deg[pos] == 0) frontier.push_back(pos);
    }
    // Deterministic ordering within each slot.
    std::sort(frontier.begin(), frontier.end());

    std::size_t layer = 0;
    while (!frontier.empty()) {
        ScheduleSlot slot;
        slot.time_step = layer;
        slot.gate_indices.reserve(frontier.size());
        for (const std::size_t pos : frontier) {
            slot.gate_indices.push_back(gate_positions[pos]);
        }
        slots.push_back(std::move(slot));

        std::vector<std::size_t> next;
        for (const std::size_t pos : frontier) {
            for (const std::size_t succ : succs[pos]) {
                if (--in_deg[succ] == 0) {
                    next.push_back(succ);
                }
            }
        }
        std::sort(next.begin(), next.end());
        frontier = std::move(next);
        ++layer;
    }

    depth_ = slots.empty() ? 0 : layer;
    return slots;
}

std::size_t EAFScheduler::logicalDepth() const noexcept { return depth_; }

} // namespace qfault
