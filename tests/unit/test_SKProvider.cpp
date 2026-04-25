#include <qfault/passes/synthesis/SKProvider.hpp>
#include <qfault/passes/synthesis/SynthesisProvider.hpp>
#include <qfault/ir/GateKind.hpp>

#include <gtest/gtest.h>

#include <complex>
#include <numbers>
#include <vector>

namespace {

static_assert(qfault::SynthesisProvider<qfault::SKProvider>,
              "SKProvider must satisfy SynthesisProvider");

using Complex = std::complex<double>;
struct Mat2 {
    Complex a, b, c, d;
    Mat2 operator*(const Mat2& o) const {
        return {a*o.a + b*o.c, a*o.b + b*o.d,
                c*o.a + d*o.c, c*o.b + d*o.d};
    }
};

constexpr double kInvSqrt2 = 0.7071067811865476;

Mat2 gateToMatrix(qfault::GateKind g) {
    using qfault::GateKind;
    const double pi = std::numbers::pi;
    auto rz = [](double t) -> Mat2 {
        Complex p = std::exp(Complex{0.0, t / 2.0});
        return {std::conj(p), 0.0, 0.0, p};
    };
    switch (g) {
        case GateKind::H:   return {kInvSqrt2, kInvSqrt2, kInvSqrt2, -kInvSqrt2};
        case GateKind::X:   return {0.0, Complex{0.0,-1.0}, Complex{0.0,-1.0}, 0.0};
        case GateKind::Y:   return {0.0, -1.0, 1.0, 0.0};
        case GateKind::Z:   return rz(pi);
        case GateKind::T:   return rz(pi / 4.0);
        case GateKind::Tdg: return rz(-pi / 4.0);
        case GateKind::S:   return rz(pi / 2.0);
        case GateKind::Sdg: return rz(-pi / 2.0);
        default:            return {1.0, 0.0, 0.0, 1.0};
    }
}

// Frobenius distance between two matrices (mod global phase).
double frobDist(const Mat2& a, const Mat2& b) {
    auto sq = [](Complex x) { return std::norm(x); };
    double d1 = std::sqrt(sq(a.a-b.a) + sq(a.b-b.b) + sq(a.c-b.c) + sq(a.d-b.d));
    double d2 = std::sqrt(sq(a.a+b.a) + sq(a.b+b.b) + sq(a.c+b.c) + sq(a.d+b.d));
    return std::min(d1, d2);
}

Mat2 composeSeq(const std::vector<qfault::GateKind>& seq) {
    Mat2 m{1.0, 0.0, 0.0, 1.0};
    for (auto g : seq) m = gateToMatrix(g) * m;
    return m;
}

bool isValidCliffordT(qfault::GateKind g) {
    using qfault::GateKind;
    switch (g) {
        case GateKind::H:
        case GateKind::X:
        case GateKind::Y:
        case GateKind::Z:
        case GateKind::S:
        case GateKind::Sdg:
        case GateKind::T:
        case GateKind::Tdg:
            return true;
        default:
            return false;
    }
}

} // namespace

TEST(SKProvider, SatisfiesSynthesisProviderConcept) {
    // Verified at compile time via static_assert above.
    SUCCEED();
}

TEST(SKProvider, NameIsCorrect) {
    qfault::SKProvider p;
    EXPECT_EQ(p.name(), "SKProvider");
}

TEST(SKProvider, SynthesisePiOver4ReturnsValidSequence) {
    qfault::SKProvider p;
    const double angle = std::numbers::pi / 4.0;
    auto seq = p.synthesise(angle, 1e-3);
    // May be empty only if identity approximation is closest — for pi/4 it should not be.
    EXPECT_FALSE(seq.empty());
}

TEST(SKProvider, AllReturnedGatesAreValidCliffordT) {
    qfault::SKProvider p;
    for (double angle : {0.0, std::numbers::pi / 4.0,
                         std::numbers::pi / 8.0, std::numbers::pi / 2.0,
                         3.0 * std::numbers::pi / 4.0}) {
        auto seq = p.synthesise(angle, 1e-3);
        for (auto g : seq) {
            EXPECT_TRUE(isValidCliffordT(g))
                << "Invalid GateKind value " << static_cast<int>(g)
                << " for angle " << angle;
        }
    }
}

TEST(SKProvider, RoundTripPiOver4WithinTolerance) {
    qfault::SKProvider p;
    const double angle = std::numbers::pi / 4.0;
    auto seq = p.synthesise(angle, 1e-3);

    const Mat2 result = composeSeq(seq);

    // Target: R_z(pi/4) in SU(2)
    auto rz = [](double t) -> Mat2 {
        Complex ph = std::exp(Complex{0.0, t / 2.0});
        return {std::conj(ph), 0.0, 0.0, ph};
    };
    const Mat2 target = rz(angle);

    const double dist = frobDist(result, target);
    EXPECT_LE(dist, 1e-3)
        << "Frobenius distance " << dist << " exceeds 1e-3 tolerance for angle pi/4";
}

TEST(SKProvider, SynthesiseZeroAngleReturnsShortSequence) {
    qfault::SKProvider p;
    // R_z(0) = identity; sequence should be short (empty or 1 gate)
    auto seq = p.synthesise(0.0, 1e-3);
    EXPECT_LE(static_cast<int>(seq.size()), 4);
}
