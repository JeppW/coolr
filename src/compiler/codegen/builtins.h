#ifndef CGEN_BUILTINS_H
#define CGEN_BUILTINS_H

#include <sstream>
#include "asm.h"
#include "classtag.h"
#include "offsets.h"
#include "../../common/consts.h"

std::string code_uninitialized_basic_objects();
std::string code_heap();
std::string code_input_buffer();
std::string code_builtin_methods();
std::string code_builtin_static_strings();
std::string code_error_procedures();
std::string code_entrypoint();
std::string code_internal_routines();

#endif