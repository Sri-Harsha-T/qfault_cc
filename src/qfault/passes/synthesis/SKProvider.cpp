#include <qfault/passes/synthesis/SKProvider.hpp>

#include <array>
#include <cmath>
#include <complex>
#include <numbers>
#include <numeric>
#include <vector>

namespace qfault {

namespace {

using Complex = std::complex<double>;

// 2×2 complex matrix (column-major: [[a,b],[c,d]])
struct Mat2 {
    Complex a, b, c, d;

    [[nodiscard]] Mat2 operator*(const Mat2& o) const noexcept {
        return {a * o.a + b * o.c, a * o.b + b * o.d,
                c * o.a + d * o.c, c * o.b + d * o.d};
    }

    // Frobenius distance squared between this and other (mod global phase).
    // We use min over the two choices of global phase ±1.
    [[nodiscard]] double distSq(const Mat2& o) const noexcept {
        auto sq = [](Complex x) { return std::norm(x); };
        double d1 = sq(a - o.a) + sq(b - o.b) + sq(c - o.c) + sq(d - o.d);
        double d2 = sq(a + o.a) + sq(b + o.b) + sq(c + o.c) + sq(d + o.d);
        return std::min(d1, d2);
    }
};

// R_z(theta) in SU(2): [[e^{-i t/2}, 0],[0, e^{i t/2}]]
[[nodiscard]] Mat2 rz(double theta) noexcept {
    const Complex phase = std::exp(Complex{0.0, theta / 2.0});
    return {std::conj(phase), 0.0, 0.0, phase};
}

constexpr double kInvSqrt2 = 0.7071067811865476;

// Gate matrices in SU(2)
constexpr Mat2 kH  = {kInvSqrt2,  kInvSqrt2,  kInvSqrt2, -kInvSqrt2};
constexpr Mat2 kX  = {0.0, Complex{0.0,-1.0}, Complex{0.0,-1.0}, 0.0};
constexpr Mat2 kY  = {0.0, -1.0, 1.0, 0.0};
constexpr Mat2 kZ  = {Complex{0.0,-1.0}, 0.0, 0.0, Complex{0.0, 1.0}};

// S = T^2, T = R_z(pi/4) up to global phase
// Using exact SU(2): T = diag(e^{-i pi/8}, e^{i pi/8})
inline Mat2 makeT() noexcept   { return rz(std::numbers::pi / 4.0); }
inline Mat2 makeTdg() noexcept { return rz(-std::numbers::pi / 4.0); }
inline Mat2 makeS() noexcept   { return rz(std::numbers::pi / 2.0); }
inline Mat2 makeSdg() noexcept { return rz(-std::numbers::pi / 2.0); }

[[nodiscard]] Mat2 gateMatrix(GateKind g) noexcept {
    switch (g) {
        case GateKind::H:   return kH;
        case GateKind::X:   return kX;
        case GateKind::Y:   return kY;
        case GateKind::Z:   return kZ;
        case GateKind::T:   return makeT();
        case GateKind::Tdg: return makeTdg();
        case GateKind::S:   return makeS();
        case GateKind::Sdg: return makeSdg();
        default:            return {1.0, 0.0, 0.0, 1.0}; // identity
    }
}

// Basic Clifford+T approximation table: sequences of increasing depth
// covering R_z rotations by multiples of pi/4 and some intermediates.
struct Entry {
    std::vector<GateKind> seq;
    Mat2                  mat;
};

// Build the basic approximation table at first use.
// Generators: {H, T, Tdg, S, Sdg} — avoids HH (identity) and TT→S, TdgTdg→Sdg
// to keep the table compact. Depth ≤ 8 gives ~10k entries after deduplication.
// We store only a representative 200-entry sample covering SU(2) roughly.
std::vector<Entry> buildTable() {
    // Seed with exact single-gate entries
    static const std::vector<GateKind> gens = {
        GateKind::H, GateKind::T, GateKind::Tdg,
        GateKind::S, GateKind::Sdg, GateKind::Z,
    };

    std::vector<Entry> table;
    table.reserve(512);

    // identity
    table.push_back({{}, {1.0, 0.0, 0.0, 1.0}});

    // BFS up to depth 7
    std::vector<Entry> frontier = table;
    for (int depth = 1; depth <= 7 && table.size() < 512; ++depth) {
        std::vector<Entry> next;
        for (const auto& e : frontier) {
            for (GateKind g : gens) {
                // Avoid trivial cancellations
                if (!e.seq.empty()) {
                    GateKind last = e.seq.back();
                    // HH = I, TT = S, TdgTdg = Sdg, SS = Z, SdgSdg = Z
                    if ((last == GateKind::H   && g == GateKind::H)   ||
                        (last == GateKind::T   && g == GateKind::T)   ||
                        (last == GateKind::Tdg && g == GateKind::Tdg) ||
                        (last == GateKind::S   && g == GateKind::S)   ||
                        (last == GateKind::T   && g == GateKind::Tdg) ||
                        (last == GateKind::Tdg && g == GateKind::T)   ||
                        (last == GateKind::S   && g == GateKind::Sdg) ||
                        (last == GateKind::Sdg && g == GateKind::S))
                        continue;
                }
                auto seq = e.seq;
                seq.push_back(g);
                auto mat = gateMatrix(g) * e.mat;
                next.push_back({std::move(seq), mat});
                if (table.size() + next.size() >= 512) goto done;
            }
        }
        done:
        for (auto& ne : next) {
            table.push_back(std::move(ne));
        }
        frontier = std::move(next);
    }

    return table;
}

const std::vector<Entry>& getTable() {
    static const std::vector<Entry> kTable = buildTable();
    return kTable;
}

[[nodiscard]] const Entry& findClosest(const Mat2& target) noexcept {
    const auto& table = getTable();
    double best = std::numeric_limits<double>::max();
    const Entry* winner = &table[0];
    for (const auto& e : table) {
        double d = e.mat.distSq(target);
        if (d < best) {
            best = d;
            winner = &e;
        }
    }
    return *winner;
}

} // namespace

std::vector<GateKind> SKProvider::synthesise(double angle, double /*eps*/) {
    // Normalize angle mod 4π (SU(2) period)
    constexpr double k4Pi = 4.0 * std::numbers::pi;
    angle = std::fmod(angle, k4Pi);
    if (angle < 0.0) angle += k4Pi;

    const Mat2 target = rz(angle);
    const Entry& best = findClosest(target);
    return best.seq;
}

} // namespace qfault
