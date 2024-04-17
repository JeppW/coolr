#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>
#include <sstream>
#include <memory>
#include <vector>
#include <map>
#include "../utils/pretty_print.h"

enum TokenType {
    // keywords go before identifiers, because they 
    // should be prioritized, i.e. "class" should 
    // be interpreted as CLASS, not OBJ_IDENTIFIER
    CLASS,
    IF,
    ELSE,
    FI,
    IN,
    INHERITS,
    LET,
    LOOP,
    POOL,
    THEN,
    WHILE,
    CASE,
    ESAC,
    OF,
    NEW,
    ISVOID,
    NOT,
    BOOL,
    INTEGER,
    STRING,  // due to the complexity of strings and string escapes,
             // strings are handled differently and don't have an associated regex
             // like the other tokens
    OBJ_IDENTIFIER,
    TYPE_IDENTIFIER,
    PLUS,
    MINUS,
    MULTIPLICATION,
    DIVISION,
    LT,
    EQ,
    LTE,
    PARENTHESIS_OPEN,
    PARENTHESIS_CLOSE,
    CURLY_BRACKET_OPEN,
    CURLY_BRACKET_CLOSE,
    COLON,
    SEMICOLON,
    DOT,
    COMMA,
    AT,
    SQUIGGLE,
    ARROW,
    ASSIGN,
    ERROR
};

// for printing tokens like in the same style
// as used in the Stanford support code 
const std::map<TokenType, std::string> token_name_mapping = {
        {TokenType::CLASS, "CLASS"},
        {TokenType::IF, "IF"},
        {TokenType::ELSE, "ELSE"},
        {TokenType::FI, "FI"},
        {TokenType::IN, "IN"},
        {TokenType::INHERITS, "INHERITS"},
        {TokenType::LET, "LET"},
        {TokenType::LOOP, "LOOP"},
        {TokenType::POOL, "POOL"},
        {TokenType::THEN, "THEN"},
        {TokenType::WHILE, "WHILE"},
        {TokenType::CASE, "CASE"},
        {TokenType::ESAC, "ESAC"},
        {TokenType::OF, "OF"},
        {TokenType::NEW, "NEW"},
        {TokenType::ISVOID, "ISVOID"},
        {TokenType::NOT, "NOT"},
        {TokenType::BOOL, "BOOL"},
        {TokenType::INTEGER, "INTEGER"},
        {TokenType::STRING, "STRING"},
        {TokenType::OBJ_IDENTIFIER, "OBJECTID"},
        {TokenType::TYPE_IDENTIFIER, "TYPEID"},
        {TokenType::PLUS, "'+'"},
        {TokenType::MINUS, "'-'"},
        {TokenType::MULTIPLICATION, "'*'"},
        {TokenType::DIVISION, "'/'"},
        {TokenType::LT, "'<'"},
        {TokenType::EQ, "'='"},
        {TokenType::LTE, "LE"},
        {TokenType::PARENTHESIS_OPEN, "'('"},
        {TokenType::PARENTHESIS_CLOSE, "')'"},
        {TokenType::CURLY_BRACKET_OPEN, "'{'"},
        {TokenType::CURLY_BRACKET_CLOSE, "'}'"},
        {TokenType::COLON, "':'"},
        {TokenType::SEMICOLON, "';'"},
        {TokenType::DOT, "'.'"},
        {TokenType::COMMA, "','"},
        {TokenType::AT, "'@'"},
        {TokenType::SQUIGGLE, "'~'"},
        {TokenType::ARROW, "DARROW"},
        {TokenType::ASSIGN, "ASSIGN"},
        {TokenType::ERROR, "ERROR"}
};

class Token {
    protected:
        TokenType type;
        uint line_number;

    public:
        Token(TokenType t, uint ln) {
            type = t;
            line_number = ln;
        }

        virtual ~Token() = default;

        TokenType get_type() {
            return type;
        }

        uint get_line_number() {
            return line_number;
        }

        virtual void dump();
        virtual void display();
};

class StringToken : public Token {
    private:
        std::string value;
    
    public:
        StringToken(const std::string& v, uint ln) : Token(TokenType::STRING, ln), value(v) {}

        std::string get_value() {
            return value;
        }

        void dump() override;
        void display() override;
};

class BoolToken : public Token {
    private:
        bool value;
    
    public:
        BoolToken(bool v, uint ln) : Token(TokenType::BOOL, ln), value(v) {}

        bool get_value() {
            return value;
        }

        void dump() override;
        void display() override;
};

class IntToken : public Token {
    private:
        std::string value;
    
    public:
        IntToken(const std::string& v, uint ln) : Token(TokenType::INTEGER, ln), value(v) {}

        std::string get_value() {
            return value;
        }

        void dump() override;
        void display() override;
};

class TypeIdToken : public Token {
    private:
        std::string value;
    
    public:
        TypeIdToken(const std::string& v, uint ln) : Token(TokenType::TYPE_IDENTIFIER, ln), value(v) {}

        std::string get_value() {
            return value;
        }

        void dump() override;
        void display() override;
};

class ObjIdToken : public Token {
    private:
        std::string value;
    
    public:
        ObjIdToken(const std::string& v, uint ln) : Token(TokenType::OBJ_IDENTIFIER, ln), value(v) {}

        std::string get_value() {
            return value;
        }

        void dump() override;
        void display() override;
};

class ErrorToken : public Token {
    private:
        std::string msg;
    
    public:
        ErrorToken(const std::string& m, uint ln) : Token(TokenType::OBJ_IDENTIFIER, ln), msg(m) {}

        std::string get_msg() {
            return msg;
        }

        void dump() override;
        void display() override;
};

class Tokenstream {
    private:
        std::vector<std::shared_ptr<Token>> tokens;
        uint pointer = 0;

    public:
        std::vector<std::shared_ptr<Token>> get_tokens();
        Token* get();
        Token* peek();
        void unget();
        void unget(uint);
        void consume();
        uint get_line_number();
        void add_token(std::shared_ptr<Token> token);
        bool eof();
};

#endif