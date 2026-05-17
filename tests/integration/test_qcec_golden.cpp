// test_qcec_golden.cpp — Issue 2.5-B-4: QCEC golden circuit regression tests.
// Reads committed circuit pairs from bench/golden/qcec/ and validates that
// check_equivalence() + is_passing() agree with the committed expected_verdict.txt.
// Guarded by QFAULT_HAS_QCEC; GTEST_SKIP() when QCEC not linked.

#include <gtest/gtest.h>

#ifdef QFAULT_HAS_QCEC

#include <qfault/oracle/QCECBridge.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

namespace fs = std::filesystem;

// Path to bench/golden/qcec/ supplied by CMake at compile time.
#ifndef QFAULT_SOURCE_DIR
#  error "QFAULT_SOURCE_DIR not set — ensure CMakeLists.txt passes it via target_compile_definitions"
#endif

static const fs::path kGoldenDir{QFAULT_SOURCE_DIR "/bench/golden/qcec"};

std::string read_file(const fs::path& p) {
    std::ifstream f{p};
    EXPECT_TRUE(f.is_open()) << "Cannot open: " << p;
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Returns the first non-empty, non-comment line (stripped of whitespace).
std::string first_token(const std::string& s) {
    std::istringstream ss{s};
    std::string line;
    while (std::getline(ss, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
            line.pop_back();
        if (!line.empty() && line.front() != '#')
            return line;
    }
    return {};
}

struct GoldenCase {
    std::string name;
    std::size_t nqubits;
};

// The 5 committed golden cases, with the qubit count used for is_passing() threshold.
// bv4=5q, bv6=7q, bv8=9q (relaxed), qft4=4q, adder4=4q.
static const GoldenCase kCases[] = {
    {"bv4",    5},
    {"bv6",    7},
    {"bv8",    9},
    {"qft4",   4},
    {"adder4", 4},
};

} // namespace

// ── Parameterised golden regression test ──────────────────────────────────────

class QCECGolden : public ::testing::TestWithParam<GoldenCase> {};

TEST_P(QCECGolden, VerdictMatchesCommittedExpectation) {
    const auto& tc  = GetParam();
    const fs::path dir = kGoldenDir / tc.name;

    const std::string qa = read_file(dir / "original.qasm");
    const std::string qb = read_file(dir / "compiled.qasm");
    const std::string expected_raw = read_file(dir / "expected_verdict.txt");
    const std::string expected = first_token(expected_raw);

    ASSERT_FALSE(qa.empty())       << "original.qasm is empty for " << tc.name;
    ASSERT_FALSE(qb.empty())       << "compiled.qasm is empty for " << tc.name;
    ASSERT_FALSE(expected.empty()) << "expected_verdict.txt is empty for " << tc.name;
    ASSERT_TRUE(expected == "passing" || expected == "not_passing")
        << "expected_verdict.txt must be 'passing' or 'not_passing', got: " << expected;

    const auto result   = qfault::oracle::check_equivalence(qa, qb, tc.nqubits);
    const bool passing  = qfault::oracle::is_passing(result, tc.nqubits);
    const bool expected_pass = (expected == "passing");

    EXPECT_EQ(passing, expected_pass)
        << "Golden case '" << tc.name << "': "
        << "QCEC verdict='" << qfault::oracle::to_string(result) << "' "
        << "is_passing=" << passing << " but expected_verdict='" << expected << "'";
}

INSTANTIATE_TEST_SUITE_P(
    GoldenCases,
    QCECGolden,
    ::testing::ValuesIn(kCases),
    [](const ::testing::TestParamInfo<GoldenCase>& info) { return info.param.name; });

#else // !QFAULT_HAS_QCEC

TEST(QCECGolden, QcecNotLinked) {
    GTEST_SKIP() << "MQT QCEC not linked — build with -DQFAULT_ENABLE_QCEC=ON (preset: gcc13-stim)";
}

#endif // QFAULT_HAS_QCEC
