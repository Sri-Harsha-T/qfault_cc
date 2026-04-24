#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

namespace qfault::frontend {

enum class TokenType {
    // Keywords
    OPENQASM, QUBIT, GATE,
    // Gate keywords (Clifford+T subset)
    H, CX, T, TDG, S, SDG, X, Y, Z, CZ, RZ, U,
    // Literals and identifiers
    IDENTIFIER, INT_LITERAL, FLOAT_LITERAL,
    // Punctuation
    LBRACKET, RBRACKET, LPAREN, RPAREN, SEMICOLON, COMMA, DOT,
    // Special
    EOF_TOKEN, UNKNOWN,
};

struct Token {
    TokenType type;
    std::string_view value;
    std::size_t line{1};
};

class Lexer {
public:
    // Returns tokens; values are views into src — src must outlive the token vector.
    [[nodiscard]] static std::vector<Token> tokenize(std::string_view src);
};

} // namespace qfault::frontend
