#ifndef LEXER_H
#define LEXER_H

#include <sstream>
#include <list>
#include <regex>
#include <map>
#include "../../common/consts.h"
#include "../../common/token.h"

// regex expressions for each type of token
static const std::map<TokenType, std::regex> patterns = {
    {TokenType::CLASS, std::regex("^class\\b", std::regex_constants::icase)},
    {TokenType::IF, std::regex("^if\\b", std::regex_constants::icase)},
    {TokenType::ELSE, std::regex("^else\\b", std::regex_constants::icase)},
    {TokenType::FI, std::regex("^fi\\b", std::regex_constants::icase)},
    {TokenType::IN, std::regex("^in\\b", std::regex_constants::icase)},
    {TokenType::INHERITS, std::regex("^inherits\\b", std::regex_constants::icase)},
    {TokenType::LET, std::regex("^let\\b", std::regex_constants::icase)},
    {TokenType::LOOP, std::regex("^loop\\b", std::regex_constants::icase)},
    {TokenType::POOL, std::regex("^pool\\b", std::regex_constants::icase)},
    {TokenType::THEN, std::regex("^then\\b", std::regex_constants::icase)},
    {TokenType::WHILE, std::regex("^while\\b", std::regex_constants::icase)},
    {TokenType::CASE, std::regex("^case\\b", std::regex_constants::icase)},
    {TokenType::ESAC, std::regex("^esac\\b", std::regex_constants::icase)},
    {TokenType::OF, std::regex("^of\\b", std::regex_constants::icase)},
    {TokenType::NEW, std::regex("^new\\b", std::regex_constants::icase)},
    {TokenType::ISVOID, std::regex("^isvoid\\b", std::regex_constants::icase)},
    {TokenType::NOT, std::regex("^not\\b", std::regex_constants::icase)},
    {TokenType::OBJ_IDENTIFIER, std::regex("^[a-z][a-zA-Z0-9_]*\\b")},
    {TokenType::TYPE_IDENTIFIER, std::regex("^[A-Z][a-zA-Z0-9_]*\\b")},
    {TokenType::INTEGER, std::regex("^[0-9]+")},
    {TokenType::BOOL, std::regex("^(t[rR][uU][eE]|f[aA][lL][sS][eE])\\b")},
    {TokenType::PLUS, std::regex("^\\+")},
    {TokenType::MINUS, std::regex("^-")},
    {TokenType::MULTIPLICATION, std::regex("^\\*")},
    {TokenType::DIVISION, std::regex("^\\/")},
    {TokenType::LT, std::regex("^<")},
    {TokenType::EQ, std::regex("^=")},
    {TokenType::LTE, std::regex("^<=")},
    {TokenType::PARENTHESIS_OPEN, std::regex("^\\(")},
    {TokenType::PARENTHESIS_CLOSE, std::regex("^\\)")},
    {TokenType::CURLY_BRACKET_OPEN, std::regex("^\\{")},
    {TokenType::CURLY_BRACKET_CLOSE, std::regex("^\\}")},
    {TokenType::COLON, std::regex("^:")},
    {TokenType::SEMICOLON, std::regex("^;")},
    {TokenType::DOT, std::regex("^\\.")},
    {TokenType::COMMA, std::regex("^,")},
    {TokenType::AT, std::regex("^@")},
    {TokenType::SQUIGGLE, std::regex("^~")},
    {TokenType::ARROW, std::regex("^=>")},
    {TokenType::ASSIGN, std::regex("^<-")}
};

static const std::regex whitespace_regex                = std::regex("^[ \t\v\r\f\n]+");
static const std::regex single_line_comment_regex       = std::regex("^--");
static const std::regex multi_line_comment_regex_open   = std::regex("^\\(\\*");
static const std::regex multi_line_comment_regex_close  = std::regex("^\\*\\)");
static const std::regex quotation_mark_regex            = std::regex("^\\\"");

enum LexState {
    DEFAULT_SCAN,
    SINGLE_LINE_COMMENT_SCAN, 
    MULTI_LINE_COMMENT_SCAN,
    STRING_SCAN,
    ESCAPED_STRING_SCAN,
    SCAN_ERROR,
    BROKEN_STRING_SCAN
};

class Scanner {
    private:
        Tokenstream token_stream;
        uint line_number = 1;
        std::string string_builder = "";
        std::string error_message;
        LexState state = DEFAULT_SCAN;

        void single_line_comment_scan(std::stringstream&);
        void multi_line_comment_scan(std::stringstream&);
        void string_scan(std::stringstream&);
        void escaped_string_scan(std::stringstream&);
        void broken_string_scan(std::stringstream&);
        void default_scan(std::stringstream&);
    
    public:
        Tokenstream scan(std::stringstream&);
};

#endif