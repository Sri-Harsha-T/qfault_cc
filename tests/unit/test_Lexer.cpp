#include <qfault/frontend/Lexer.hpp>

#include <gtest/gtest.h>
#include <string>

using namespace qfault::frontend;

TEST(Lexer, EmptySourceProducesOnlyEOF) {
    auto toks = Lexer::tokenize("");
    ASSERT_EQ(toks.size(), 1u);
    EXPECT_EQ(toks[0].type, TokenType::EOF_TOKEN);
}

TEST(Lexer, OpenQASMHeader) {
    // "3.0" is a single FLOAT_LITERAL token, not three separate tokens
    auto toks = Lexer::tokenize("OPENQASM 3.0;");
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[0].type, TokenType::OPENQASM);
    EXPECT_EQ(toks[1].type, TokenType::FLOAT_LITERAL);
    EXPECT_EQ(toks[1].value, "3.0");
    EXPECT_EQ(toks[2].type, TokenType::SEMICOLON);
}

TEST(Lexer, QubitDeclaration) {
    auto toks = Lexer::tokenize("qubit[2] q;");
    EXPECT_EQ(toks[0].type, TokenType::QUBIT);
    EXPECT_EQ(toks[1].type, TokenType::LBRACKET);
    EXPECT_EQ(toks[2].type, TokenType::INT_LITERAL);
    EXPECT_EQ(toks[2].value, "2");
    EXPECT_EQ(toks[3].type, TokenType::RBRACKET);
    EXPECT_EQ(toks[4].type, TokenType::IDENTIFIER);
    EXPECT_EQ(toks[4].value, "q");
    EXPECT_EQ(toks[5].type, TokenType::SEMICOLON);
}

TEST(Lexer, GateKeywords) {
    auto toks = Lexer::tokenize("h cx t tdg s sdg x y z cz");
    EXPECT_EQ(toks[0].type, TokenType::H);
    EXPECT_EQ(toks[1].type, TokenType::CX);
    EXPECT_EQ(toks[2].type, TokenType::T);
    EXPECT_EQ(toks[3].type, TokenType::TDG);
    EXPECT_EQ(toks[4].type, TokenType::S);
    EXPECT_EQ(toks[5].type, TokenType::SDG);
    EXPECT_EQ(toks[6].type, TokenType::X);
    EXPECT_EQ(toks[7].type, TokenType::Y);
    EXPECT_EQ(toks[8].type, TokenType::Z);
    EXPECT_EQ(toks[9].type, TokenType::CZ);
}

TEST(Lexer, GateApplication) {
    auto toks = Lexer::tokenize("h q[0];");
    EXPECT_EQ(toks[0].type, TokenType::H);
    EXPECT_EQ(toks[1].type, TokenType::IDENTIFIER);
    EXPECT_EQ(toks[1].value, "q");
    EXPECT_EQ(toks[2].type, TokenType::LBRACKET);
    EXPECT_EQ(toks[3].type, TokenType::INT_LITERAL);
    EXPECT_EQ(toks[3].value, "0");
    EXPECT_EQ(toks[4].type, TokenType::RBRACKET);
    EXPECT_EQ(toks[5].type, TokenType::SEMICOLON);
}

TEST(Lexer, TwoQubitGate) {
    auto toks = Lexer::tokenize("cx q[0], q[1];");
    EXPECT_EQ(toks[0].type, TokenType::CX);
    EXPECT_EQ(toks[3].type, TokenType::INT_LITERAL);
    EXPECT_EQ(toks[4].type, TokenType::RBRACKET);
    EXPECT_EQ(toks[5].type, TokenType::COMMA);
    EXPECT_EQ(toks[6].type, TokenType::IDENTIFIER);
}

TEST(Lexer, LineCommentSkipped) {
    auto toks = Lexer::tokenize("// this is a comment\nh q[0];");
    EXPECT_EQ(toks[0].type, TokenType::H);
}

TEST(Lexer, LineNumberTracked) {
    auto toks = Lexer::tokenize("h q[0];\ncx q[0], q[1];");
    EXPECT_EQ(toks[0].line, 1u); // h
    // cx is on line 2
    std::size_t cxIdx = 0;
    for (std::size_t i = 0; i < toks.size(); ++i) {
        if (toks[i].type == TokenType::CX) { cxIdx = i; break; }
    }
    EXPECT_EQ(toks[cxIdx].line, 2u);
}

TEST(Lexer, UnknownTokenProducesUnknownType) {
    auto toks = Lexer::tokenize("@");
    EXPECT_EQ(toks[0].type, TokenType::UNKNOWN);
    EXPECT_EQ(toks[0].value, "@");
}

TEST(Lexer, FullCircuit) {
    const std::string src = "OPENQASM 3.0; qubit[2] q; h q[0]; cx q[0], q[1]; t q[1];";
    auto toks = Lexer::tokenize(src);
    // Just verify the last real token before EOF is SEMICOLON
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks.back().type, TokenType::EOF_TOKEN);
    EXPECT_EQ(toks[toks.size() - 2].type, TokenType::SEMICOLON);
}
