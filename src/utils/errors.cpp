#include "errors.h"

/*
 *  Utility functions for outputting error messages.
 *
 *  All error functions exit the program after printing the error message. 
 */

void parser_error(Tokenstream& ts, Token* token) {
    // make error messages similar to Flex/Bison
    // for compatibility with the Stanford grading tests
    if (token == nullptr) {
        // handle special case where token is EOF
        std::cout << "Line " << ts.get_line_number() << ": ";
        std::cout << "syntax error at or near EOF" << std::endl;
    } else {
        std::cout << "Line " << token->get_line_number() << ": ";
        std::cout << "syntax error at or near ";
        token->display();
    }

    std::cout << "Compilation halted due to lex and parse errors" << std::endl;
    exit(1);
}

void semant_error(const std::string& msg, int line_no) {
    std::cout << "Line " << line_no << ": " << msg << std::endl;
    std::cout << "Compilation halted due to static semantic errors." << std::endl;
    exit(1);
}

void semant_error(const std::string& msg) {
    std::cout << msg << std::endl;
    std::cout << "Compilation halted due to static semantic errors." << std::endl;
    exit(1);
}