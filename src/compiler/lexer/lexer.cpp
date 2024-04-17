#include "lexer.h"

/*
 *  Lexical analyzer.
 *  
 *  This file implements a lexer using a state machine
 *  and returns a Tokenstream object.
 */

void Scanner::single_line_comment_scan(std::stringstream& program) {
    for (;;) {
        if (program.get() == '\n') {
            line_number++;
            state = DEFAULT_SCAN;
            return;
        }

        if (program.eof()) {
            return;
        }
    }
}

void Scanner::multi_line_comment_scan(std::stringstream& program) {
    uint cursor = program.tellg();
    std::string program_str = program.str();

    // COOL supports nested multi-line comments
    // this variable keeps track of the number of nested comments
    uint nested_count = 1;

    std::smatch m;

    for (;;) {
        if (program.eof()) {
            error_message = "EOF in comment";
            token_stream.add_token(std::make_shared<ErrorToken>(error_message, line_number));
            return;
        }

        std::string next = program_str.substr(cursor);

        if (std::regex_search(next, m, multi_line_comment_regex_open)) {
            nested_count++;
            cursor += m.str().length();
        } else if (std::regex_search(next, m, multi_line_comment_regex_close)) {
            nested_count--;
            cursor += m.str().length();
        } else if (program_str[cursor++] == '\n') {
            line_number++;
        }

        if (nested_count == 0) {
            program.seekg(cursor);
            state = DEFAULT_SCAN;
            return;
        }

        program.seekg(cursor); program.get();  // trigger eof
    }
}

void Scanner::string_scan(std::stringstream& program) {
    for (;;) {
        char c = program.get();

        if (program.eof()) {
            error_message = "EOF in string constant";
            token_stream.add_token(std::make_shared<ErrorToken>(error_message, line_number));
            return;
        }

        if (string_builder.length() + 1 > Constants::MaxStringSize) {
            error_message = "String constant too long";
            string_builder = "";
            state = BROKEN_STRING_SCAN;
            return;
        }

        if (c == '"') {
            StringToken token = StringToken(string_builder, line_number);
            token_stream.add_token(std::make_shared<StringToken>(string_builder, line_number));
            string_builder = "";
            state = DEFAULT_SCAN;
            return;
        } else if (c == '\\') {
            state = ESCAPED_STRING_SCAN;
            return;
        } else if (c == '\n') {
            line_number++;
            string_builder = "";
            error_message = "Unterminated string constant";
            state = SCAN_ERROR;
            return;
        } else if (c == '\0') {
            error_message = "String contains null character.";
            state = BROKEN_STRING_SCAN;
            return;
        } else {
            string_builder += c;
        }
    }
}

void Scanner::escaped_string_scan(std::stringstream& program) {
    char c = program.get();

    if (program.eof()) {
        error_message = "EOF in string constant";
        token_stream.add_token(std::make_shared<ErrorToken>(error_message, line_number));
        return;
    }

    switch (c) {
        case '\0':
            error_message = "String contains escaped null character.";
            state = BROKEN_STRING_SCAN;
            return;
        case '\n':
            line_number++;
            string_builder += "\n";
            break;
        case 'n':
            string_builder += "\n";
            break;
        case 't':
            string_builder += "\t";
            break;
        case 'b':
            string_builder += "\b";
            break;
        case 'f':
            string_builder += "\f";
            break;
        default:
            string_builder += c;
            break;
    }

    state = STRING_SCAN;
}

void Scanner::broken_string_scan(std::stringstream& program) {
    // error state for recovering from broken strings
    // this state doesn't output anything, it just consumes input
    // and returns to the default state when approriate
    bool escaped = false;
    
    for (;;) {
        char c = program.get();
        switch (c) {
            case '\\':
                escaped = true;
                break;
            case '\n':
                line_number++;
                if (!escaped) {
                    state = DEFAULT_SCAN;
                    return;
                }
                break;
            case '"':
                state = DEFAULT_SCAN;
                return;
            default:
                escaped = false;
                break;
        }
    }
}

void Scanner::default_scan(std::stringstream& program) {
    int cursor = program.tellg();
    std::string program_str = program.str();
    std::string next = program_str.substr(cursor);

    // ignore whitespace 
    std::smatch whitespace_match;
    if (std::regex_search(next, whitespace_match, whitespace_regex)) {
        std::string match_str = whitespace_match.str();
        line_number += std::count(match_str.begin(), match_str.end(), '\n');
        program.seekg(cursor += match_str.length());
        next = program_str.substr(cursor);
    }

    // first, check for strings and comments
    // these are the characters that cause a state transition
    std::smatch m;
    if (std::regex_search(next, m, single_line_comment_regex)) {
        state = SINGLE_LINE_COMMENT_SCAN; 
        program.seekg(cursor + m.str().length());
        return;
    } else if (std::regex_search(next, m, multi_line_comment_regex_open)) {
        state = MULTI_LINE_COMMENT_SCAN;
        program.seekg(cursor + m.str().length());
        return;
    } else if (std::regex_search(next, m, quotation_mark_regex)) {
        state = STRING_SCAN;
        program.seekg(cursor + m.str().length());
        return;
    } else if (std::regex_search(next, m, multi_line_comment_regex_close)) {
        error_message = "Unmatched *)";
        state = SCAN_ERROR;
        program.seekg(cursor + m.str().length());
        return;
    }

    // otherwise, identify tokens as normal
    std::list<std::pair<std::smatch, TokenType> > matches;

    // find all matches
    for (auto pattern : patterns) {
        std::regex regex = pattern.second;
        std::smatch match;

        if (std::regex_search(next, match, regex)) {
            matches.push_back(std::make_pair(match, pattern.first));
        }
    }

    // handle invalid token
    if (matches.empty()) {
        // no match found
        error_message = program.get();
        state = SCAN_ERROR;
        return;
    }

    // get the longest match 
    auto token_match = std::max_element(matches.begin(), matches.end(), 
        [](const auto& a, const auto& b) {
            int len_a = a.first.str().length();
            int len_b = b.first.str().length();

            // note: token type will be used as a tie-breaker
            return len_a < len_b;
        });


    // add token to token stream
    std::smatch match = token_match->first;
    TokenType token_type = token_match->second;
    int length = match.str().length();

    switch (token_type) {
        case TokenType::BOOL: {
            bool value = match.str()[0] == 't';
            token_stream.add_token(std::make_shared<BoolToken>(value, line_number));
            break;
        }

        case TokenType::INTEGER: {
            token_stream.add_token(std::make_shared<IntToken>(match.str(), line_number));
            break;
        }

        case TokenType::TYPE_IDENTIFIER: {
            token_stream.add_token(std::make_shared<TypeIdToken>(match.str(), line_number));
            break;
        }

        case TokenType::OBJ_IDENTIFIER: {
            token_stream.add_token(std::make_shared<ObjIdToken>(match.str(), line_number));
            break;
        }

        default: {
            token_stream.add_token(std::make_shared<Token>(token_type, line_number));
            break;
        }
    }

    program.seekg(cursor + length);
}

Tokenstream Scanner::scan(std::stringstream& program) {
    while (!program.eof()) {

        switch (state) {
            case SINGLE_LINE_COMMENT_SCAN:
                single_line_comment_scan(program);
                break;

            case MULTI_LINE_COMMENT_SCAN:
                multi_line_comment_scan(program);
                break;

            case STRING_SCAN:
                string_scan(program);
                break;
            
            case ESCAPED_STRING_SCAN:
                escaped_string_scan(program);
                break;
            
            case BROKEN_STRING_SCAN:
                token_stream.add_token(std::make_shared<ErrorToken>(error_message, line_number));            
                broken_string_scan(program);
                break;
            
            case SCAN_ERROR:
                token_stream.add_token(std::make_shared<ErrorToken>(error_message, line_number));
                state = DEFAULT_SCAN;
                break;

            default:
                default_scan(program);
                break;
        }
    }

    return this->token_stream;
}