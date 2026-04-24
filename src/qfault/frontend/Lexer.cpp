#include <qfault/frontend/Lexer.hpp>

#include <cctype>

namespace qfault::frontend {

namespace {

TokenType classifyWord(std::string_view word) {
    if (word == "OPENQASM") return TokenType::OPENQASM;
    if (word == "qubit")    return TokenType::QUBIT;
    if (word == "gate")     return TokenType::GATE;
    if (word == "h")        return TokenType::H;
    if (word == "cx")       return TokenType::CX;
    if (word == "t")        return TokenType::T;
    if (word == "tdg")      return TokenType::TDG;
    if (word == "s")        return TokenType::S;
    if (word == "sdg")      return TokenType::SDG;
    if (word == "x")        return TokenType::X;
    if (word == "y")        return TokenType::Y;
    if (word == "z")        return TokenType::Z;
    if (word == "cz")       return TokenType::CZ;
    if (word == "rz")       return TokenType::RZ;
    if (word == "u")        return TokenType::U;
    return TokenType::IDENTIFIER;
}

} // namespace

std::vector<Token> Lexer::tokenize(std::string_view src) {
    std::vector<Token> tokens;
    std::size_t pos = 0;
    std::size_t line = 1;

    while (pos < src.size()) {
        const char c = src[pos];

        if (c == '\n') { ++line; ++pos; continue; }
        if (std::isspace(static_cast<unsigned char>(c))) { ++pos; continue; }

        // Line comments
        if (c == '/' && pos + 1 < src.size() && src[pos + 1] == '/') {
            while (pos < src.size() && src[pos] != '\n') ++pos;
            continue;
        }

        // Single-character punctuation
        switch (c) {
            case '[': tokens.push_back({TokenType::LBRACKET,  src.substr(pos, 1), line}); ++pos; continue;
            case ']': tokens.push_back({TokenType::RBRACKET,  src.substr(pos, 1), line}); ++pos; continue;
            case '(': tokens.push_back({TokenType::LPAREN,    src.substr(pos, 1), line}); ++pos; continue;
            case ')': tokens.push_back({TokenType::RPAREN,    src.substr(pos, 1), line}); ++pos; continue;
            case ';': tokens.push_back({TokenType::SEMICOLON, src.substr(pos, 1), line}); ++pos; continue;
            case ',': tokens.push_back({TokenType::COMMA,     src.substr(pos, 1), line}); ++pos; continue;
            case '.': tokens.push_back({TokenType::DOT,       src.substr(pos, 1), line}); ++pos; continue;
            default: break;
        }

        // Numeric literals
        if (std::isdigit(static_cast<unsigned char>(c))) {
            const std::size_t start = pos;
            while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos]))) ++pos;
            bool isFloat = false;
            if (pos < src.size() && src[pos] == '.') {
                isFloat = true;
                ++pos;
                while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos]))) ++pos;
            }
            tokens.push_back({
                isFloat ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL,
                src.substr(start, pos - start),
                line,
            });
            continue;
        }

        // Identifiers and keywords
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            const std::size_t start = pos;
            while (pos < src.size() &&
                   (std::isalnum(static_cast<unsigned char>(src[pos])) || src[pos] == '_')) {
                ++pos;
            }
            const std::string_view word = src.substr(start, pos - start);
            tokens.push_back({classifyWord(word), word, line});
            continue;
        }

        tokens.push_back({TokenType::UNKNOWN, src.substr(pos, 1), line});
        ++pos;
    }

    tokens.push_back({TokenType::EOF_TOKEN, {}, line});
    return tokens;
}

} // namespace qfault::frontend
