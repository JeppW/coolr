#include "token.h"

/*
 *  Tokenstream methods.
 */

std::vector<std::shared_ptr<Token>> Tokenstream::get_tokens() {
    return tokens;
}

Token* Tokenstream::get() {
    // get the next token and move the pointer
    if (pointer < tokens.size()) {
        return tokens[pointer++].get();
    }

    return nullptr;
};

Token* Tokenstream::peek() {
    // get the next token without consuming it
    if (pointer < tokens.size()) {
        return tokens[pointer].get();
    }

    return nullptr;
};

// the unget methods move the pointer back
// useful for backtracking
void Tokenstream::unget() {
    pointer--;
}

void Tokenstream::unget(uint n) {
    pointer -= n;
}

void Tokenstream::consume() {
    // move the pointer to the next token
    if (pointer < tokens.size()) {
        pointer++;
        return;
    }
};

uint Tokenstream::get_line_number() {
    // get the line number of the token which was last consumed
    if (pointer >= 1) {
        if (pointer <= tokens.size()) {
            return tokens[pointer-1].get()->get_line_number();
        } else {
            tokens.back()->get_line_number();
        }
    }
    return 1;
};

bool Tokenstream::eof() {
    return pointer >= tokens.size();
}

void Tokenstream::add_token(std::shared_ptr<Token> token) {
    tokens.push_back(std::move(token));
}


/*
 *  Display methods for dumping tokens.
 *
 *  The display methods follow the syntax used in the Stanford course
 *  so that this compiler can be tested using the grading tests from the course.
 */

void Token::dump() {
    std::string token_name;
    auto it = token_name_mapping.find(type);

    if (it != token_name_mapping.end()) {
        token_name = it->second;
    } else {
        token_name = "UNKNOWN";
    }

    std::cout << "#" << line_number << " "
              << token_name << std::endl;
}

void StringToken::dump() {
    std::cout << "#" << line_number
              << " STR_CONST ";

    std::cout << get_pretty_string(get_value()) << std::endl;
}

void BoolToken::dump() {
    std::string str_value = value ? "true" : "false";
    std::cout << "#" << line_number
              << " BOOL_CONST " << str_value
              << std::endl;
}

void IntToken::dump() {
    std::cout << "#" << line_number
              << " INT_CONST " << value 
              << std::endl;
}

void TypeIdToken::dump() {
    std::cout << "#" << line_number
              << " TYPEID " << value
              << std::endl;
}

void ObjIdToken::dump() {
    std::cout << "#" << line_number
              << " OBJECTID " << value
              << std::endl;
}

void ErrorToken::dump() {
    std::cout << "#" << line_number
              << " ERROR \"";
    
    for (char c : msg) {
        if (c == '\\') {
            std::cout << "\\\\";
        } else if (std::isprint(c)) {
            std::cout << c;
        } else {
            std::cout << "\\" << std::setw(3) << std::setfill('0') << uint(c);
        }
    }

    std::cout << '"' << std::endl;
}

void Token::display() {
    std::string token_name;
    auto it = token_name_mapping.find(type);

    if (it != token_name_mapping.end()) {
        token_name = it->second;
    } else {
        token_name = "UNKNOWN";
    }

    std::cout << token_name << std::endl;
}

void StringToken::display() {
    std::cout << "STR_CONST = " << get_pretty_string(get_value()) << std::endl;
}

void BoolToken::display() {
    std::string str_value = value ? "true" : "false";
    std::cout << "BOOL_CONST = " << str_value << std::endl;
}

void IntToken::display() {
    std::cout << "INT_CONST = " << value << std::endl;
}

void TypeIdToken::display() {
    std::cout << "TYPEID = " << value << std::endl;
}

void ObjIdToken::display() {
    std::cout << "OBJECTID = " << value << std::endl;
}

void ErrorToken::display() {
    std::cout << "ERROR = " << get_pretty_string(get_msg()) << std::endl;
}