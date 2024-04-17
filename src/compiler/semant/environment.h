#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <string>
#include <map>
#include "../../common/ast.h"

class ObjectScope {
    private:
        std::map<std::string, std::string> objects;

    public:
        std::map<std::string, std::string> get_objects();
        void add_object(const std::string&, const std::string&);
        bool exists(const std::string&);
        std::string get_object(const std::string&);
};

class ObjectEnv {
    private:
        std::vector<ObjectScope*> scopes;
    
    public:
        void enter_scope();
        void exit_scope();
        void add_object(const std::string&, const std::string&);
        bool probe(const std::string&);
        std::string lookup(const std::string&);
};

class MethodEnv {
    private:
        // map a class name and method name pair to a MethodNode
        std::map<std::pair<std::string, std::string>, MethodNode*> methods;

    public:
        MethodNode* find(const std::string& cls, const std::string&);
        void set(const std::string&, MethodNode*);
        bool exists(const std::string& cls, const std::string&);
};

class TypeEnvironment {
    public:
        ObjectEnv objects;
        MethodEnv methods;
        ClassNode* cls;
};

#endif