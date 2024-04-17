#include "codegen.h"

/*
 *  Code generation module.
 *
 *  The code generation module is responsible for generating the assembly code
 *  for the COOL program using the AST and the class table.
 */

static std::ofstream outfile;
static ProgramNode* ast;
static ClassTable* classtable;
static ScopeStack scope_stack;

static uint string_counter = 0;
static std::map<std::string, std::string> strings;

static std::string current_class = "";

template<typename T>
std::string unique_label(const std::string& name, const T& ptr) {
    // use the address of the object to generate a unique label
    std::stringstream ss;
    ss << name << "_" << ptr;
    return ss.str();
}

template<typename T>
void make_new_int_object(const T& value) {
    // allocate and return new Bool object
    outfile << Asm::push(value);
    outfile << Asm::replace_selfptr("Int_proto");
    outfile << Asm::call("Object.copy");
    outfile << Asm::restore_selfptr();
    outfile << Asm::pop(ebx);
    outfile << Asm::mov(dword_ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)), ebx);
}

template<typename T>
void make_new_bool_object(const T& value) {
    // allocate and return new Bool object
    outfile << Asm::push(value);
    outfile << Asm::replace_selfptr("Bool_proto");
    outfile << Asm::call("Object.copy");
    outfile << Asm::restore_selfptr();
    outfile << Asm::pop(ebx);
    outfile << Asm::mov(ptr(eax, get_attr_offset(Strings::Types::Bool, Strings::Attributes::Val)), ebx);
}

uint calculate_obj_size(ClassNode* cls) {
    uint size = Constants::NumObjHeaders;

    for (const std::string& clsname : classtable->get_ancestry(cls->get_name())) {
        ClassNode* inherited_class = classtable->clsmap[clsname];
        size += inherited_class->get_attributes().size();
    }

    return size * Constants::WordSize;
}

void build_class_prototypes() {
    outfile << Asm::label(selfptr);
    outfile << Asm::dd(0);
    outfile << Asm::newline();

    for (auto it = classtable->clsmap.begin(); it != classtable->clsmap.end(); ++it) {
        std::string clsname = it->first;
        ClassNode* cls = it->second;

        outfile << Asm::comment("class " + clsname);
        outfile << Asm::label(clsname + "_proto");

        // unique class tag
        outfile << Asm::dd(get_class_tag(clsname));

        // typename 
        outfile << Asm::dd(clsname + "_typename");
        strings[cls->get_name() + "_typename"] = cls->get_name();

        // object size = (number of attributes + number of headers) * word size
        outfile << Asm::dd(calculate_obj_size(cls));

        // dispatch pointer  
        outfile << Asm::dd(clsname + "_dispatch_table");

        // parent class
        if (clsname == Strings::Types::Object) {
            outfile << Asm::dd(0);  // Object has no parent
        } else {
            outfile << Asm::dd(cls->get_base_class() + "_proto");
        }

        uint count = Constants::NumObjHeaders; // account for the headers in the offset calculations
        std::vector<std::string> ancestry = classtable->get_ancestry(clsname);
        
        for (auto it = ancestry.rbegin(); it != ancestry.rend(); ++it) {
            std::string clsname = *it;
            ClassNode* inherited_class = classtable->clsmap[clsname];

            if (clsname == Strings::Types::String) {
                // handle String object as a special case:
                // use simple int (not Int object) as val and
                // empty_string as str_field
                set_attr_offset(clsname, Strings::Attributes::Val, 4 * count++);
                outfile << Asm::comment("attribute val");
                outfile << Asm::dd(0);
                set_attr_offset(clsname, Strings::Attributes::StrField, 4 * count++);
                outfile << Asm::comment("attribute str_field");
                outfile << Asm::dd(empty_string);
                continue;
            }

            for (AttributeNode* attr : inherited_class->get_attributes()) {
                // inherited attributes cannot be redefined -
                // no need to check for overriding
                set_attr_offset(clsname, attr->get_name(), 4 * count++);
                outfile << Asm::comment("attribute " + attr->get_name());
                if (attr->get_type() == Strings::Types::String) {
                    outfile << Asm::dd(uninitialized_string);
                } else if (attr->get_type() == Strings::Types::Int) {
                    outfile << Asm::dd(uninitialized_int);
                } else if (attr->get_type() == Strings::Types::Bool) {
                    outfile << Asm::dd(uninitialized_bool);
                } else {
                    // other classes are just void
                    outfile << Asm::dd(0);
                }
            }
        }

        outfile << std::endl;
    }

    outfile << code_uninitialized_basic_objects();
}

void print_dispatch_tables() {
    outfile << Asm::comment("dispatch tables");

    for (auto it = classtable->clsmap.begin(); it != classtable->clsmap.end(); ++it) {
        std::string clsname = it->first;
        outfile << Asm::label(clsname + "_dispatch_table");

        std::vector<std::pair<std::string, std::string>> methods;
        std::vector<std::string> ancestry = classtable->get_ancestry(clsname);
        
        for (auto it = ancestry.rbegin(); it != ancestry.rend(); ++it) {
            std::string clsname = *it;
            ClassNode* inherited_class = classtable->clsmap[clsname];
            for (MethodNode* method : inherited_class->get_methods()) {
                // check for overriding
                bool overridden = false;
                for (size_t i = 0; i < methods.size(); ++i) {
                    if (method->get_name() == methods[i].second) {
                        methods[i] = std::make_pair(clsname, method->get_name());
                        overridden = true;
                        break;
                    }
                }
                
                if (!overridden) {
                    methods.push_back(std::make_pair(clsname, method->get_name()));
                }
            }
        }

        // add the internal _init function to the dispatch table
        outfile << Asm::dd(clsname + "._init");

        uint count = 1;
        for (std::pair<std::string, std::string> method : methods) {
            outfile << Asm::dd(method.first + "." + method.second);
            set_method_offset(clsname, method.second, 4 * count++);
        }

        outfile << std::endl;
    }
}

void print_string_constants() {
    outfile << Asm::comment("string constants");

    for (auto string : strings) {
        std::string label = string.first;
        std::string value = string.second;
        outfile << Asm::static_string(label, value);
    }

    outfile << code_builtin_static_strings();
}

void print_heap() {
    outfile << code_heap();
}

void print_input_buffer() {
    outfile << code_input_buffer();
}

void code_initializers() {
    // initializers for each class
    outfile << Asm::comment("internal initializer methods");
    
    for (ClassNode* cls : ast->get_classes()) {
        std::string type = cls->get_name();
        outfile << Asm::label(type + "._init");
        
        // get prototype
        outfile << Asm::mov(eax, type + "_proto");

        // get size and allocate memory
        outfile << Asm::mov(ebx, ptr(eax, 8));
        outfile << Asm::push(eax);
        outfile << Asm::push(ebx);
        outfile << Asm::call("_allocate_memory");

        // copy the prototype to the newly allocated memory
        outfile << Asm::mov(edi, eax);
        outfile << Asm::pop(esi);
        outfile << Asm::mov(ecx, ptr(esi, 8));
        outfile << Asm::cld();
        outfile << Asm::rep_movsb();

        // evaluate initializers
        // switch to new class so we use its dispatch table as offset
        std::string old_class = current_class;
        current_class = cls->get_name();
        outfile << Asm::replace_selfptr(eax);
        outfile << Asm::push(eax);

        // add all the attributes to the scope
        // (attributes may use other attributes in their initialization)        
        std::vector<std::string> ancestry = classtable->get_ancestry(cls->get_name());
        uint offset = Constants::NumObjHeaders;

        for (auto it = ancestry.rbegin(); it != ancestry.rend(); ++it) {
            std::string clsname = *it;
            ClassNode* cls = classtable->clsmap[clsname];
            for (AttributeNode* attr : cls->get_attributes()) {
                scope_stack.add_attribute(attr->get_name(), Constants::WordSize * offset++);
            }
        }

        // initialize the attributes
        for (auto it = ancestry.rbegin(); it != ancestry.rend(); ++it) {
            std::string clsname = *it;
            ClassNode* cls = classtable->clsmap[clsname];
            for (AttributeNode* attr : cls->get_attributes()) {
                outfile << Asm::comment("evaluate initializer " + attr->get_name());
                
                // make a clean temporary stack frame free from the init stuff on the stack
                // for evaluating attributes initializers
                outfile << Asm::enter();
                attr->get_expr()->code();
                outfile << Asm::leave();

                outfile << Asm::pop(edi);
                outfile << Asm::mov(ptr(edi, get_attr_offset(cls->get_name(), attr->get_name())), eax);
                outfile << Asm::push(edi);
            }
        }

        // return address of new object
        current_class = old_class;
        outfile << Asm::pop(eax);
        outfile << Asm::restore_selfptr();
        outfile << Asm::ret();
        outfile << Asm::newline();
    }

    // the built-in classes are special (with prim_slot and all)
    for (const std::string& cls : { Strings::Types::Object, 
                             Strings::Types::Int, 
                             Strings::Types::Bool, 
                             Strings::Types::String, 
                             Strings::Types::IO 
                           }) {
        uint attr_num = classtable->clsmap[cls]->get_attributes().size();

        outfile << Asm::label(cls + "._init");
        outfile << Asm::push((Constants::NumObjHeaders + attr_num) * Constants::WordSize);
        outfile << Asm::call("_allocate_memory");
        outfile << Asm::push(eax);
        outfile << Asm::mov(edi, eax);
        outfile << Asm::mov(esi, cls + "_proto");
        outfile << Asm::mov(ecx, (Constants::NumObjHeaders + attr_num) * Constants::WordSize);
        outfile << Asm::cld();
        outfile << Asm::rep_movsb();
        outfile << Asm::pop(eax);
        outfile << Asm::ret();
        outfile << Asm::newline();
    }
}

void build_text_segment() {
    outfile << Asm::global("_start");
    outfile << Asm::newline();

    // built-in methods
    outfile << Asm::comment("built-in methods");
    outfile << code_builtin_methods();

    // initializers for each class
    code_initializers();

    // user-defined methods
    outfile << Asm::comment("user-defined methods");
    for (ClassNode* cls : ast->get_classes()) {
        for (MethodNode* method : cls->get_methods()) {
            // setup scope for each method
            current_class = cls->get_name();
            scope_stack.enter_scope();
            Scope* scope = scope_stack.get_scope();

            std::vector<std::string> ancestry = classtable->get_ancestry(cls->get_name());
            for (auto it = ancestry.rbegin(); it != ancestry.rend(); ++it) {
                std::string clsname = *it;
                ClassNode* cls = classtable->clsmap[clsname];
                for (AttributeNode* attr : cls->get_attributes()) {
                    uint offset = get_attr_offset(cls->get_name(), attr->get_name());
                    scope_stack.add_attribute(attr->get_name(), offset);
                }
            }

            std::vector<FormalNode*> formals = method->get_formals()->get_formals();
            for (auto it = formals.rbegin(); it != formals.rend(); ++it) {
                FormalNode* formal = *it;
                scope->add_parameter(formal->get_name());
            }

            // generate code for method
            outfile << Asm::label(cls->get_name() + "." + method->get_name());
            outfile << Asm::enter();
            method->get_expr()->code();
            outfile << Asm::leave();

            // clean up dispatch parameters
            outfile << Asm::ret(method->get_formals()->get_formals().size() * Constants::WordSize);
            outfile << Asm::newline();

            scope_stack.exit_scope();
        }
    }

    // internals
    outfile << code_internal_routines();

    // init Main and set selfptr to the new Main instance
    // call Main.main and execute cleanly afterwards
    outfile << code_entrypoint();

    // special exit functions for run-time errors
    outfile << code_error_procedures();
}


void generate_code(ProgramNode& program, std::string filename, ClassTable* c) {
    outfile.open(filename);
    ast = &program;
    classtable = c;

    scope_stack.enter_scope();

    // build first data segment
    // objects and dispatch tables
    outfile << Asm::data_section_start();
    build_class_prototypes(); 
    print_dispatch_tables();

    // build text segment
    outfile << Asm::text_section_start();
    build_text_segment();

    // build second data segment
    // static strings, heap and I/O buffer
    outfile << Asm::data_section_start();
    print_string_constants();
    print_heap();
    print_input_buffer();
}

void NoExpressionNode::code() {
    std::string type = get_declared_type();

    if (type == Strings::Types::String 
        || type == Strings::Types::Int 
        || type == Strings::Types::Bool
    ) {
        outfile << Asm::replace_selfptr(type + "_proto");
        outfile << Asm::call("Object.copy");
        outfile << Asm::restore_selfptr();
    } else {
        // non-basic objects are void by default
        outfile << Asm::mov(eax, 0);
    }
}

void IntNode::code() {
    make_new_int_object(get_value());
}

void StringNode::code() {
    // register the string value so it added to the .data section
    std::string string_label = "string_" + std::to_string(string_counter++);
    strings[string_label] = get_escaped_string(get_value());

    outfile << Asm::replace_selfptr("String_proto");
    outfile << Asm::call("Object.copy");
    outfile << Asm::restore_selfptr();
    outfile << Asm::mov(ebx, eax);
    outfile << Asm::add(eax, get_attr_offset(Strings::Types::String, Strings::Attributes::StrField));
    outfile << Asm::mov(dword_ptr(eax), string_label);
    outfile << Asm::sub(eax, 4);
    outfile << Asm::push(eax);
    outfile << Asm::push(string_label);
    outfile << Asm::call("_strlen");
    outfile << Asm::pop(ebx);
    outfile << Asm::mov(ptr(ebx), eax);
    outfile << Asm::lea(eax, ptr(ebx, -Constants::NumObjHeaders * Constants::WordSize));
}

void BoolNode::code() {
    make_new_bool_object(get_value());
}

void IdentifierNode::code() {
    // retrieve object from scope
    outfile << scope_stack.get_location(get_name()) << std::endl;
    outfile << Asm::mov(eax, ptr(eax));
}

void AssignmentNode::code() {
    // evaluate expression and store it in the object
    get_expr()->code();
    outfile << Asm::push(eax);
    outfile << Asm::mov(ebx, eax);
    outfile << scope_stack.get_location(get_name()) << std::endl;
    outfile << Asm::mov(ptr(eax), ebx);
    outfile << Asm::pop(eax);
}

void NewNode::code() {
    // call the _init method of the class
    std::string type = get_type();
    
    if (type == Strings::Types::SelfType) {
        // if type is 'SELF_TYPE', we have to get 
        // the type of the current 'self' object
        outfile << Asm::mov(eax, ptr(selfptr));
        outfile << Asm::mov(eax, ptr(eax, 12));
        outfile << Asm::mov(eax, ptr(eax));
        outfile << Asm::call(eax);
    } else {
        outfile << Asm::call(type + "._init");
    }
}

void IsvoidNode::code() {
    // return a boolean indicating 
    // whether the object is a null pointer
    get_expr()->code();
    outfile << Asm::cmp(eax, 0);
    outfile << Asm::setz(al);
    outfile << Asm::movzx(eax, al);
    make_new_bool_object(eax);
}

void NegNode::code() {
    // retrieve the integer value and negate it
    get_expr()->code();
    outfile << Asm::add(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val));
    outfile << Asm::mov(eax, ptr(eax));
    outfile << Asm::neg(eax);
    make_new_int_object(eax);
}

void ComplementNode::code() {
    // retrieve the boolean (1 or 0) value and xor with 1
    get_expr()->code();
    outfile << Asm::add(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val));
    outfile << Asm::mov(eax, ptr(eax));
    outfile << Asm::xor_(eax, 1);
    make_new_bool_object(eax);
}

// when evaluating binary expressions, we first evaluate the left
// push it onto the stack, evaluate the right, pop the left from 
// the stack, and then perform the operation

void PlusNode::code() {
    get_first()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)));
    outfile << Asm::push(eax);
    get_second()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)));
    outfile << Asm::pop(ebx);
    outfile << Asm::add(eax, ebx);
    make_new_int_object(eax);
}

void MinusNode::code() {
    get_first()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)));
    outfile << Asm::push(eax);
    get_second()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)));
    outfile << Asm::pop(ebx);
    outfile << Asm::sub(ebx, eax);
    outfile << Asm::mov(eax, ebx);
    make_new_int_object(eax);
}

void MultiplicationNode::code() {
    get_first()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)));
    outfile << Asm::push(eax);
    get_second()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)));
    outfile << Asm::pop(ebx);
    outfile << Asm::imul(ebx);
    make_new_int_object(eax);
}

void DivisionNode::code() {
    get_first()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)));
    outfile << Asm::push(eax);
    get_second()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)));
    outfile << Asm::pop(ebx);
    outfile << Asm::xchg(eax, ebx);
    outfile << Asm::xor_(edx, edx);
    outfile << Asm::div(ebx);
    make_new_int_object(eax);
}

void LTNode::code() {
    get_first()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)));
    outfile << Asm::push(eax);
    get_second()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)));
    outfile << Asm::pop(ebx);
    outfile << Asm::cmp(eax, ebx);
    outfile << Asm::setg(al);
    outfile << Asm::movzx(eax, al);
    make_new_bool_object(eax);
}

void LTENode::code() {
    get_first()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)));
    outfile << Asm::push(eax);
    get_second()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val)));
    outfile << Asm::pop(ebx);
    outfile << Asm::cmp(eax, ebx);
    outfile << Asm::setge(al);
    outfile << Asm::movzx(eax, al);
    make_new_bool_object(eax);
}

void EQNode::code() {
    outfile << "  ; equals expression" << std::endl;
    std::string type = get_first()->get_checked_type();

    if (type == Strings::Types::String) {
        // comparing strings in assembly is different than comparing
        // bools and integers - we have to implement something
        // similar to C's strcmp()
        get_first()->code();
        outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::String, Strings::Attributes::StrField)));
        outfile << Asm::push(eax);
        get_second()->code();
        outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::String, Strings::Attributes::StrField)));
        outfile << Asm::push(eax);
        outfile << Asm::call("_strcmp");
    } else if (type == Strings::Types::Int || type == Strings::Types::Bool) {
        get_first()->code();
        outfile << Asm::mov(eax, ptr(eax, get_attr_offset(type, Strings::Attributes::Val)));
        outfile << Asm::push(eax);
        get_second()->code();
        outfile << Asm::mov(eax, ptr(eax, get_attr_offset(type, Strings::Attributes::Val)));
        outfile << Asm::pop(ebx);
        outfile << Asm::cmp(eax, ebx);
        outfile << Asm::setz(al);
        outfile << Asm::movzx(eax, al);
        make_new_bool_object(eax);
    } else {
        // object equality: test if pointers are identical
        get_first()->code();
        outfile << Asm::push(eax);
        get_second()->code();
        outfile << Asm::pop(ebx);
        outfile << Asm::cmp(eax, ebx);
        outfile << Asm::setz(al);
        outfile << Asm::movzx(eax, al);
        make_new_bool_object(eax);
    }
}

void ConditionalNode::code() {
    get_predicate()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Bool, Strings::Attributes::Val)));
    outfile << Asm::test(eax, eax);

    // if the value of the predicate is not zero, jump to the 'then' branch
    outfile << Asm::jne(unique_label(".cond_true", this));
    outfile << Asm::label(unique_label(".cond_false", this));
    get_else()->code();
    outfile << Asm::jmp(unique_label(".cond_over", this));
    outfile << Asm::label(unique_label(".cond_true", this));
    get_then()->code();
    outfile << Asm::label(unique_label(".cond_over", this));
}

void WhileNode::code() {
    // execute the body in a loop until the predicate is false
    outfile << Asm::label(unique_label(".while_begin", this));
    get_predicate()->code();
    outfile << Asm::mov(eax, ptr(eax, get_attr_offset(Strings::Types::Bool, Strings::Attributes::Val)));
    outfile << Asm::test(eax, eax);
    outfile << Asm::je(unique_label(".while_end", this));
    get_body()->code();
    outfile << Asm::jmp(unique_label(".while_begin", this));
    outfile << Asm::label(unique_label(".while_end", this));
    outfile << Asm::xor_(eax, eax);  // loops return void
}

void BlockNode::code() {
    // simply evaluate all the expressions in order
    for (ExpressionNode* expression : get_expressions()) {
        expression->code();
    }
}

void CaseNode::code() {   
    uint i;
    get_target()->code();

    // first, check if expr0 evaluates to void;
    // if so, produce a run-time error
    outfile << Asm::cmp(eax, 0);
    outfile << Asm::je("_match_on_void");
    outfile << Asm::push(eax);  // add expr0 as a stack variable 

    outfile << Asm::label(unique_label(".case_branch_start", this));
    outfile << Asm::mov(ecx, ptr(eax));  // load classtag into eax

    i = 0;
    for (CaseBranchNode* branch : get_branches()) {
        outfile << Asm::mov(ebx, ptr(branch->get_type() + "_proto"));
        outfile << Asm::cmp(ecx, ebx);
        outfile << Asm::je(unique_label(".case_branch_" + std::to_string(i++), this));
    }

    // recursively repeat with parent class until we reach Object
    // if that happens and no branch was taken, generate a run-time error
    outfile << Asm::mov(eax, ptr(eax, 16));
    outfile << Asm::cmp(eax, 0);  // only Object has '0' as parent class
    outfile << Asm::je(unique_label(".case_branch_error", this));
    outfile << Asm::jmp(unique_label(".case_branch_start", this));

    i = 0;
    for (CaseBranchNode* branch : get_branches()) {
        // bind expression value to branch identifier
        scope_stack.enter_scope();
        scope_stack.add_stack_variable(branch->get_name());

        outfile << Asm::label(unique_label(".case_branch_" + std::to_string(i++), this));
        branch->get_expr()->code();
        outfile << Asm::jmp(unique_label(".case_finish", this));

        scope_stack.exit_scope();
    }

    // if no case matched, produce a run-time error
    outfile << Asm::label(unique_label(".case_branch_error", this));
    outfile << Asm::jmp("_no_match");

    outfile << Asm::label(unique_label(".case_finish", this));
    outfile << Asm::add(esp, 4);  // remove the expr0 stack variable
}

void LetNode::code() {
    ExpressionNode* body = get_body();
    std::vector<LetInitializerNode*> initializers = get_initializers();

    scope_stack.enter_scope();

    // evaluate the initializers and add them to the scope
    for (LetInitializerNode* initializer : initializers) {
        initializer->code();
    }

    body->code();

    scope_stack.exit_scope();
    outfile << Asm::add(esp, initializers.size() * Constants::WordSize);
}

void LetInitializerNode::code() {
    get_expr()->code();
    outfile << Asm::push(eax);
    scope_stack.add_stack_variable(get_name());
}

void DispatchNode::code() {
    ExpressionNode* object = get_object();
    std::string object_type = object->get_checked_type();
    std::vector<ExpressionNode*> parameters = get_parameters();

    if (object_type == Strings::Types::SelfType) {
        object_type = current_class;
    }

    // save the old selfptr
    outfile << Asm::mov(eax, ptr(selfptr));
    outfile << Asm::push(eax);
    
    // pass the dispatch arguments in order
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        ExpressionNode* parameter = *it;
        parameter->code();
        outfile << Asm::push(eax);
    }

    // evaluate the object of the dispatch and save it
    object->code();

    // check if object is void
    // it is an error to dispatch an a void object
    outfile << Asm::cmp(eax, 0);
    outfile << Asm::je("_dispatch_to_void");

    // save the caller object pointer
    outfile << Asm::mov(ebx, eax);

    // get pointer to dispatch table of the object
    outfile << Asm::mov(eax, ptr(eax, 12));
    
    // get the correct entry in the dispatch table
    outfile << Asm::mov(eax, ptr(eax, get_method_offset(object_type, get_method_name())));

    // overwrite the selfptr and execute the dispatch
    std::string old_class = current_class;
    current_class = object_type;
    outfile << Asm::mov(ptr(selfptr), ebx);
    outfile << Asm::call(eax);
    current_class = old_class;

    // restore the selfptr
    outfile << Asm::pop(ebx);
    outfile << Asm::mov(ptr(selfptr), ebx);
}

void StaticDispatchNode::code() {
    std::string static_type = get_static_type();
    ExpressionNode* object = get_object();
    std::string object_type = object->get_checked_type();

    // save the old selfptr
    outfile << Asm::mov(eax, ptr(selfptr));
    outfile << Asm::push(eax);
    
    // pass the dispatch arguments in order
    std::vector<ExpressionNode*> parameters = get_parameters();
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        ExpressionNode* parameter = *it;
        parameter->code();
        outfile << Asm::push(eax);
    }

    // evaluate the object of the dispatch and save it
    object->code();

    // check if object is void
    // it is an error to dispatch an a void object
    outfile << Asm::cmp(eax, 0);
    outfile << Asm::je("_dispatch_to_void");

    // save the caller object pointer
    outfile << Asm::mov(ebx, eax);

    // get the correct entry in the dispatch table of the specified static type
    outfile << Asm::mov(eax, ptr(static_type + "_dispatch_table", get_method_offset(static_type, get_method_name())));

    // overwrite the selfptr and execute the dispatch
    std::string old_class = current_class;
    current_class = object_type;
    outfile << Asm::mov(ptr(selfptr), ebx);
    outfile << Asm::call(eax);
    current_class = old_class;

    // restore the selfptr
    outfile << Asm::pop(ebx);
    outfile << Asm::mov(ptr(selfptr), ebx);
}