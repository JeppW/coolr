#ifndef SCOPE_H
#define SCOPE_H

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include "asm.h"
#include "../../common/ast.h"
#include "../../common/consts.h"

// objects: attributes, method parameters, let statements and case statements

class Scope {
    private:
        uint stack_base = 0;
        uint stack_offset = 0;
        uint method_argument_counter = 0;
        std::vector<std::pair<std::string, std::string>> objects;

    public:
        Scope(uint basis) : stack_base(basis) {}
        void add_stack_variable(const std::string&);
        void add_parameter(const std::string&);
        void add_attribute(const std::string&, uint);
        bool exists(const std::string&);
        std::string get_location(const std::string&);

        uint get_stack_offset() {
            return stack_offset;
        }

        uint get_method_argument_counter() {
            return method_argument_counter;
        }
};

class ScopeStack {
    private:
        std::vector<Scope*> scopes;
        uint stack_var_counter = 0;
    
    public:
        void enter_scope();
        void exit_scope();
        Scope* get_scope();
        void add_stack_variable(const std::string&);
        void add_parameter(const std::string&);
        void add_attribute(const std::string&, uint);
        std::string get_location(const std::string&);
};

#endif