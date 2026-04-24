#pragma once

#include <chrono>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace qfault {

enum class DiagLevel { Info, Warn, Error };

struct Diagnostic {
    DiagLevel level{DiagLevel::Info};
    std::string message;
};

class PassContext {
public:
    // codeDistance must be odd and >= 3; throws std::invalid_argument otherwise.
    explicit PassContext(unsigned codeDistance, double synthesisEpsilon = 1e-10)
        : codeDistance_{codeDistance}, synthesisEpsilon_{synthesisEpsilon} {
        if (codeDistance_ % 2 == 0 || codeDistance_ < 3) {
            throw std::invalid_argument(
                "codeDistance must be odd and >= 3 (got " +
                std::to_string(codeDistance_) + ")");
        }
    }

    [[nodiscard]] unsigned codeDistance() const noexcept { return codeDistance_; }
    [[nodiscard]] double synthesisEpsilon() const noexcept { return synthesisEpsilon_; }

    void addDiagnostic(DiagLevel lvl, std::string_view msg) {
        diagnostics_.push_back({lvl, std::string{msg}});
    }

    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept {
        return diagnostics_;
    }

    void startTimer(std::string_view /*passName*/) noexcept {
        timerStart_ = std::chrono::steady_clock::now();
    }

    void stopTimer() noexcept {
        lastDuration_ = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - timerStart_);
    }

    [[nodiscard]] std::chrono::microseconds lastPassDuration() const noexcept {
        return lastDuration_;
    }

private:
    unsigned codeDistance_;
    double synthesisEpsilon_;
    std::vector<Diagnostic> diagnostics_;
    std::chrono::steady_clock::time_point timerStart_;
    std::chrono::microseconds lastDuration_{0};
};

} // namespace qfault
