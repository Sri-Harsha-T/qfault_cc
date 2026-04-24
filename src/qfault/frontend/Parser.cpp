#include <qfault/frontend/Parser.hpp>
#include <qfault/frontend/Lexer.hpp>
#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/LogicalQubit.hpp>

#include <optional>
#include <string>

namespace qfault::frontend {

namespace {

class ParseSession {
public:
    explicit ParseSession(std::vector<Token> tokens)
        : tokens_(std::move(tokens)) {}

    ParseResult run();

private:
    [[nodiscard]] const Token& peek() const { return tokens_[pos_]; }
    const Token& advance() { return tokens_[pos_++]; }
    [[nodiscard]] bool at(TokenType t) const { return peek().type == t; }
    [[nodiscard]] bool atEnd() const { return at(TokenType::EOF_TOKEN); }

    void skipToSemicolon() {
        while (!atEnd() && !at(TokenType::SEMICOLON)) advance();
        if (at(TokenType::SEMICOLON)) advance();
    }

    void addError(std::string msg) {
        diag_.push_back({DiagLevel::Error, std::move(msg)});
        skipToSemicolon();
    }

    void addWarn(std::string msg) {
        diag_.push_back({DiagLevel::Warn, std::move(msg)});
    }

    static std::optional<GateKind> toGateKind(TokenType t) {
        switch (t) {
            case TokenType::H:   return GateKind::H;
            case TokenType::CX:  return GateKind::CX;
            case TokenType::T:   return GateKind::T;
            case TokenType::TDG: return GateKind::Tdg;
            case TokenType::S:   return GateKind::S;
            case TokenType::SDG: return GateKind::Sdg;
            case TokenType::X:   return GateKind::X;
            case TokenType::Y:   return GateKind::Y;
            case TokenType::Z:   return GateKind::Z;
            case TokenType::CZ:  return GateKind::CZ;
            default:             return std::nullopt;
        }
    }

    // OPENQASM version ;
    void parseHeader() {
        if (!at(TokenType::OPENQASM)) return;
        advance();
        while (!atEnd() && !at(TokenType::SEMICOLON)) advance();
        if (at(TokenType::SEMICOLON)) advance();
    }

    // qubit [ N ] name ;
    void parseQubitDecl() {
        advance(); // consume 'qubit'
        if (!at(TokenType::LBRACKET)) { addError("expected '[' after qubit"); return; }
        advance();
        if (!at(TokenType::INT_LITERAL)) { addError("expected integer size in qubit declaration"); return; }
        std::size_t count = 0;
        for (char ch : peek().value) count = count * 10 + static_cast<std::size_t>(ch - '0');
        advance();
        if (!at(TokenType::RBRACKET)) { addError("expected ']' after qubit size"); return; }
        advance();
        if (!at(TokenType::IDENTIFIER)) { addError("expected qubit register name"); return; }
        std::string regName{peek().value};
        advance();
        if (!at(TokenType::SEMICOLON)) { addError("expected ';' after qubit declaration"); return; }
        advance();
        for (std::size_t i = 0; i < count; ++i) {
            module_.qubits.push_back(LogicalQubit{.name = regName, .index = i});
        }
    }

    // name [ index ]  →  LogicalQubit; returns nullopt on syntax error
    std::optional<LogicalQubit> parseOperand() {
        if (!at(TokenType::IDENTIFIER)) {
            addError("expected qubit register name");
            return std::nullopt;
        }
        std::string regName{peek().value};
        advance();
        if (!at(TokenType::LBRACKET)) { addError("expected '[' in qubit operand"); return std::nullopt; }
        advance();
        if (!at(TokenType::INT_LITERAL)) { addError("expected integer index in qubit operand"); return std::nullopt; }
        std::size_t idx = 0;
        for (char ch : peek().value) idx = idx * 10 + static_cast<std::size_t>(ch - '0');
        advance();
        if (!at(TokenType::RBRACKET)) { addError("expected ']' in qubit operand"); return std::nullopt; }
        advance();
        // Look up in declared qubits; allow undeclared (with a warning)
        for (const auto& q : module_.qubits) {
            if (q.name == regName && q.index == idx) return q;
        }
        addWarn("qubit " + regName + "[" + std::to_string(idx) + "] not declared");
        return LogicalQubit{.name = regName, .index = idx};
    }

    // gatename operands... ;
    void parseGate() {
        const auto kind = toGateKind(peek().type);
        const std::string gateName{peek().value};
        advance();

        if (!kind) {
            addWarn("unsupported gate '" + gateName + "', skipping");
            skipToSemicolon();
            return;
        }

        std::vector<LogicalQubit> operands;
        auto op = parseOperand();
        if (!op) return; // error recovery already done
        operands.push_back(*op);

        while (at(TokenType::COMMA)) {
            advance();
            op = parseOperand();
            if (!op) return;
            operands.push_back(*op);
        }

        if (!at(TokenType::SEMICOLON)) { addError("expected ';' after gate"); return; }
        advance();

        module_.instructions.push_back(LogicalGate{
            .kind = *kind,
            .operands = std::move(operands),
        });
    }

    std::vector<Token> tokens_;
    std::size_t pos_{0};
    QFaultIRModule module_;
    std::vector<Diagnostic> diag_;
};

ParseResult ParseSession::run() {
    module_.level = IRLevel::LOGICAL;
    parseHeader();

    while (!atEnd()) {
        if (at(TokenType::QUBIT)) {
            parseQubitDecl();
        } else if (at(TokenType::GATE)) {
            // gate definitions not supported in this subset — skip the body
            addWarn("gate definitions not supported in Stage 1 subset, skipping");
            // gate bodies use braces; skip to next '}' then any trailing ';'
            while (!atEnd() && !at(TokenType::SEMICOLON)) advance();
            if (at(TokenType::SEMICOLON)) advance();
        } else if (toGateKind(peek().type).has_value()) {
            parseGate();
        } else if (at(TokenType::IDENTIFIER)) {
            addWarn("unknown statement '" + std::string{peek().value} + "', skipping");
            skipToSemicolon();
        } else {
            advance(); // skip unrecognised token
        }
    }

    return {std::move(module_), std::move(diag_)};
}

} // namespace

ParseResult Parser::parse(std::string_view src) {
    auto tokens = Lexer::tokenize(src);
    return ParseSession{std::move(tokens)}.run();
}

} // namespace qfault::frontend
