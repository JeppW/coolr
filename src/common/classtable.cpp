#include "classtable.h"

/*
 *   The class table provides a way to retrieve and store class information
 *   by class name. It also checks for errors in the class definitions
 *   and validates the class hierarchy.
 */

static std::ostringstream error_msg;

void ClassTable::install_basic_classes() {
    // Object class
    ClassNode* object_class = new ClassNode(Strings::Types::Object);
    MethodNode* abort = new MethodNode(Strings::Methods::Abort);
    abort->set_formals(new FormalsNode());
    abort->set_type(Strings::Types::Object);
    MethodNode* type_name = new MethodNode(Strings::Methods::TypeName);
    type_name->set_formals(new FormalsNode());
    type_name->set_type(Strings::Types::String);
    MethodNode* copy = new MethodNode(Strings::Methods::Copy);
    copy->set_formals(new FormalsNode());
    copy->set_type(Strings::Types::SelfType);
    object_class->add_method(abort);
    object_class->add_method(type_name);
    object_class->add_method(copy);

    // IO class
    ClassNode* io_class = new ClassNode(Strings::Types::IO);
    MethodNode* out_string = new MethodNode(Strings::Methods::OutString);
    FormalsNode* out_string_formals = new FormalsNode();
    out_string_formals->add_formal(new FormalNode(Strings::Parameters::Arg, Strings::Types::String));
    out_string->set_formals(out_string_formals);
    out_string->set_type(Strings::Types::SelfType);
    MethodNode* out_int = new MethodNode(Strings::Methods::OutInt);
    FormalsNode* out_int_formals = new FormalsNode();
    out_int_formals->add_formal(new FormalNode(Strings::Parameters::Arg, Strings::Types::Int));
    out_int->set_formals(out_int_formals);
    out_int->set_type(Strings::Types::SelfType);
    MethodNode* in_string = new MethodNode(Strings::Methods::InString);
    in_string->set_formals(new FormalsNode());
    in_string->set_type(Strings::Types::String);
    MethodNode* in_int = new MethodNode(Strings::Methods::InInt);
    in_int->set_formals(new FormalsNode());
    in_int->set_type(Strings::Types::Int);
    io_class->set_base_class(Strings::Types::Object);
    io_class->add_method(out_string);
    io_class->add_method(out_int);
    io_class->add_method(in_string);
    io_class->add_method(in_int);

    // Int class
    ClassNode* int_class = new ClassNode(Strings::Types::Int);
    AttributeNode* int_val = new AttributeNode(Strings::Attributes::Val);
    int_val->set_type(Strings::Types::PrimSlot);
    int_class->set_base_class(Strings::Types::Object);
    int_class->add_attribute(int_val);

    // Bool class
    ClassNode* bool_class = new ClassNode(Strings::Types::Bool);
    AttributeNode* bool_val = new AttributeNode(Strings::Attributes::Val);
    bool_val->set_type(Strings::Types::PrimSlot);
    bool_class->set_base_class(Strings::Types::Object);
    bool_class->add_attribute(bool_val);

    // String class
    ClassNode* string_class = new ClassNode(Strings::Types::String);
    AttributeNode* str_val = new AttributeNode(Strings::Attributes::Val);
    str_val->set_type(Strings::Types::Int);
    AttributeNode* str_field = new AttributeNode(Strings::Attributes::StrField);
    str_field->set_type(Strings::Types::PrimSlot);
    MethodNode* length = new MethodNode(Strings::Methods::Length);
    length->set_formals(new FormalsNode());
    length->set_type(Strings::Types::Int);
    MethodNode* concat = new MethodNode(Strings::Methods::Concat);
    FormalsNode* concat_formals = new FormalsNode();
    concat_formals->add_formal(new FormalNode(Strings::Parameters::Arg, Strings::Types::String));
    concat->set_formals(concat_formals);
    concat->set_type(Strings::Types::String);
    MethodNode* substr = new MethodNode(Strings::Methods::Substr);
    FormalsNode* substr_formals = new FormalsNode();
    substr_formals->add_formal(new FormalNode(Strings::Parameters::Arg1, Strings::Types::Int));
    substr_formals->add_formal(new FormalNode(Strings::Parameters::Arg2, Strings::Types::Int));
    substr->set_formals(substr_formals);
    substr->set_type(Strings::Types::String);
    string_class->set_base_class(Strings::Types::Object);
    string_class->add_attribute(str_val);
    string_class->add_attribute(str_field);
    string_class->add_method(length);
    string_class->add_method(concat);
    string_class->add_method(substr);

    clsmap[Strings::Types::Object] = object_class;
    clsmap[Strings::Types::IO] = io_class;
    clsmap[Strings::Types::Int] = int_class;
    clsmap[Strings::Types::Bool] = bool_class;
    clsmap[Strings::Types::String] = string_class;
}

ClassTable::ClassTable(std::vector<ClassNode*> classes) {
    install_basic_classes();

    // iterate over the classes, identifying multiply defined classes
    // add classes to a map, so we can find a class by symbol later
    for (ClassNode* cls : classes) {
        std::string name = cls->get_name();
        std::string parent = cls->get_base_class();

        // basic classes must not be redefined
        if (name == Strings::Types::Int || name == Strings::Types::String || name == Strings::Types::Bool || name == Strings::Types::IO || name == Strings::Types::Object) {
            error_msg << "Redefinition of basic class " << name << ".";
            semant_error(error_msg.str(), cls->get_line_number());
        }

        // special case: the SELF_TYPE class must not be redefined
        if (name == Strings::Types::SelfType) {
            error_msg << "Redefinition of basic class " << name << ".";
            semant_error(error_msg.str(), cls->get_line_number());
        };

        // classes must not be multiply defined
        if (clsmap.find(name) != clsmap.end()) {
            error_msg << "Class " << name << " was previously defined.";
            semant_error(error_msg.str(), cls->get_line_number());
        }

        // it is an error to inherit from Int, Str and Bool
        if (parent == Strings::Types::Int 
            || parent == Strings::Types::String 
            || parent == Strings::Types::Bool 
            || parent == Strings::Types::SelfType
        ) {
            error_msg << "Class " << cls->get_name() << " cannot inherit class " << parent << ".";
            semant_error(error_msg.str(), cls->get_line_number());
        }

        // if we get this far, the class is fine and can be added to the map
        clsmap[name] = cls;
    }

    // the Main class must be defined
    if (clsmap.find(Strings::Types::MainClass) == clsmap.end()) {
        semant_error("Class Main is not defined.");
    }

    ClassNode* main_class = clsmap.find(Strings::Types::MainClass)->second;

    // verify that the Main class contains a method feature called "main"
    bool main_method_exists = false;
    std::vector<MethodNode*> methods = main_class->get_methods();
    
    for (MethodNode* method : methods) {
        if (method->get_name() == Strings::Methods::MainMethod) {
            main_method_exists = true;
            break;
        }
    }    

    if (!main_method_exists) {
        error_msg << "No main() method defined in Main.";
        semant_error(error_msg.str(), main_class->get_line_number());
    }

    // all good! classes look fine, now let's check the inheritance graph
    // after that, initialization is done
    check_inheritance_graph();
}

void ClassTable::check_inheritance_graph() {
    // we don't have to actually "build" an inheritance graph;
    // using the get_base_class method of the ClassNode object, we 
    // already have an adjacency list representation

    // verify that all parent classes actually exists
    for (auto iter = clsmap.begin(); iter != clsmap.end(); ++iter) {
        std::string name = iter->first;
        ClassNode* cls = iter->second;

        if (name == Strings::Types::Object) {
            // the Object class is the root, so this test
            // doesn't apply to it
            continue;
        }

        std::string parent = cls->get_base_class();

        if (clsmap.find(parent) == clsmap.end()) {
            // the parent class is not defined
            error_msg << "Class " << name
                      << " inherits from an undefined class " << parent << ".";
            semant_error(error_msg.str(), cls->get_line_number());
        }
    }

    // check for cycles in the inheritance graph
    for (auto iter = clsmap.begin(); iter != clsmap.end(); ++iter) {
        std::string name = iter->first;
        ClassNode* cls = iter->second;

        std::string ancestor_symbol = name;
        ClassNode* ancestor_class = cls;

        while (ancestor_symbol != Strings::Types::Object) {
            ancestor_symbol = ancestor_class->get_base_class();

            // if we find an ancestor that matches the name
            // of the original class, we have a cycle
            if (ancestor_symbol == name) {
                error_msg << "Class " << name << " directly or indirectly inherits from itself.";
                semant_error(error_msg.str(), cls->get_line_number());
            }

            ancestor_class = clsmap[ancestor_symbol];
        }
    }
}

bool ClassTable::exists(const std::string& cls) {
    return clsmap.find(cls) != clsmap.end();
}

// helper method for getting all parents of a class
std::vector<std::string> ClassTable::get_ancestry(const std::string& cls) {
    std::vector<std::string> ancestry;

    std::string node = cls;
    while (node != Strings::Types::Object) {
        ancestry.push_back(node);
        node = clsmap[node]->get_base_class();
    }

    ancestry.push_back(Strings::Types::Object);

    return ancestry;
}

// simple LUB implementation: get ancestry for both classes
// and return the first class present in both ancestries 
std::string ClassTable::least_upper_bound(const std::string& a, const std::string& b) {
    std::vector<std::string> ancestry_a = get_ancestry(a);
    std::vector<std::string> ancestry_b = get_ancestry(b);

    for (size_t i = 0; i < ancestry_a.size(); ++i) {
        for (size_t j = 0; j < ancestry_b.size(); ++j) {
            if (ancestry_a[i] == ancestry_b[j]) {
                return ancestry_a[i];
            }
        }
    }

    // if nothing else, we know that the two classes
    // must both be a child of Object
    return Strings::Types::Object;
}

// overloaded method for supporting vectors of arbitrary length 
std::string ClassTable::least_upper_bound(std::vector<std::string> symbols) {
    std::string lub = symbols[0];

    for (size_t i = 0; i < symbols.size(); ++i) {
        lub = least_upper_bound(lub, symbols[i]);
    }

    return lub;
}