#ifndef ERRORS_H
#define ERRORS_H

#include <string>
#include <iostream>
#include "../common/token.h"

void parser_error(Tokenstream&, Token*);

void semant_error(const std::string&, int);
void semant_error(const std::string&);

#endif