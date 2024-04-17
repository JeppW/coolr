#include "environment.h"

/*
 *  Implementation of the type environment methods.
 */

void ObjectScope::add_object(const std::string& name, const std::string& type) {
    // add object to the scope
    objects[name] = type;
}

bool ObjectScope::exists(const std::string& name) {
    // check if object exists in the scope
    return objects.find(name) != objects.end();
}

std::string ObjectScope::get_object(const std::string& name) {
    // get the type of the object
    if (exists(name)) {
        return objects.find(name)->second;
    } else {
        return "";
    }
}

void ObjectEnv::enter_scope() {
    scopes.push_back(new ObjectScope());
}

void ObjectEnv::exit_scope() {
    scopes.pop_back();
}

void ObjectEnv::add_object(const std::string& name, const std::string& type) {
    // add object to the current scope
    scopes.back()->add_object(name, type);
}

bool ObjectEnv::probe(const std::string& name) {
    // check if object is defined in current scope
    return scopes.back()->exists(name);
}

std::string ObjectEnv::lookup(const std::string& name) {
    // return the closest definition of the object
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        ObjectScope* scope = *it;
        if (scope->exists(name)) {
            return scope->get_object(name);
        }
    }

    return "";
}

MethodNode* MethodEnv::find(const std::string& cls, const std::string& method) {
    return methods[std::make_pair(cls, method)];
}

void MethodEnv::set(const std::string& cls, MethodNode* method) {
    methods[std::make_pair(cls, method->get_name())] = method;
}

bool MethodEnv::exists(const std::string& cls, const std::string& method) {
    return methods.find(std::make_pair(cls, method)) != methods.end();
}