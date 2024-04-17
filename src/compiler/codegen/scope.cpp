#include "scope.h"

/*  
 *  Module for keeping track of in-scope variables.
 *  When an object is added to the scope, assembly code for recovering the object is saved.
 *  That way, objects can be retrieved from the scope in a uniform manner regardless of whether
 *  they are attributs, method parameters, or let/case statement variables.  
 */

void Scope::add_stack_variable(const std::string& name) {
    // stack variables are stored in the stack frame above the base pointer
    // the stack grows downwards, so the offset is negative
    std::string code = Asm::lea(eax, ptr(ebp, -(Constants::WordSize * (++stack_offset + stack_base))));
    objects.push_back(std::make_pair(name, code));
}

void Scope::add_parameter(const std::string& name) {
    // method parameters are stored below the base pointer
    // we add 1 to the offset to account for the return address
    std::string code = Asm::lea(eax, ptr(ebp, Constants::WordSize * (++method_argument_counter + 1)));
    objects.push_back(std::make_pair(name, code));
}

void Scope::add_attribute(const std::string& name, uint offset) {
    // attributes are located at a fixed offset from the self pointer
    std::string code = Asm::mov(eax, ptr(selfptr)) + Asm::add(eax, offset);
    objects.push_back(std::make_pair(name, code));
}

bool Scope::exists(const std::string& name) {
    for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
        if (it->first == name) {
            return true;
        }
    }

    if (name == Strings::Self) {
        return true;
    }

    return false;
}

std::string Scope::get_location(const std::string& name) {
    for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
        if (it->first == name) {
            return it->second;
        }
    }

    if (name == Strings::Self) {
        return Asm::lea(eax, ptr(selfptr));
    }

    throw std::logic_error("Error: Requested object not found in scope.");
}

void ScopeStack::enter_scope() {
    scopes.push_back(new Scope(stack_var_counter));
}

void ScopeStack::exit_scope() {
    stack_var_counter -= scopes.back()->get_stack_offset();
    scopes.pop_back();
}

Scope* ScopeStack::get_scope() {
    return scopes.back();
}

void ScopeStack::add_stack_variable(const std::string& name) {
    scopes.back()->add_stack_variable(name);
    stack_var_counter++;
}

void ScopeStack::add_parameter(const std::string& name) {
    scopes.back()->add_parameter(name);
}

void ScopeStack::add_attribute(const std::string& name, uint offset) {
    scopes.back()->add_attribute(name, offset);
}

std::string ScopeStack::get_location(const std::string& variable) {
    // return the closest definition of the object
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        Scope* scope = *it;
        if (scope->exists(variable)) {
            return scope->get_location(variable);
        }
    }

    throw std::logic_error("Error: Requested object not found in scope.");
}