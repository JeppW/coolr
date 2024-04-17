#ifndef CODEGEN_H
#define CODEGEN_H

#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include "asm.h"
#include "classtag.h"
#include "scope.h"
#include "offsets.h"
#include "builtins.h"
#include "../../common/ast.h"
#include "../../common/classtable.h"
#include "../../common/consts.h"
#include "../../utils/pretty_print.h"

void generate_code(ProgramNode&, std::string, ClassTable*);

#endif