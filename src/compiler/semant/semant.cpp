#include "semant.h"

/*
 *  Semantic analyzer.
 *
 *  The semantic analyzer implements the 'analyze' and 'typecheck' 
 *  methods of the nodes in the AST. The methods call the relevant methods
 *  of their children, such that the entire AST is recursively traversed
 *  and type-checked upon calling 'analyze' on the root node.
 */

static ClassTable* classtable;
static std::ostringstream error_msg;

// helper method for resolving SELF_TYPE 
// to the name of the current env.cls
std::string resolve(const std::string& type, TypeEnvironment& env) {
    if (type == Strings::Types::SelfType) {
        return env.cls->get_name();
    } else {
        return type;
    }
}

void add_class_to_method_env(ClassNode* cls, std::string cls_name, TypeEnvironment& env) {
    if (cls->get_name() != Strings::Types::Object) {
        // classes inherit all methods from their parents,
        // so this function is called recursively until we reach Object
        // note that we keep class name the same - the parent methods are added to THIS class!
        ClassNode* parent = classtable->clsmap[cls->get_base_class()];
        add_class_to_method_env(parent, cls_name, env);
    }

    // set for keeping track of added methods,
    // used for checking for multiple defined methods
    std::unordered_set<std::string> added_methods;

    // add all the class methods to the environment
    for (MethodNode* method : cls->get_methods()) {
        std::string method_name = method->get_name();
        FormalsNode* formals = method->get_formals();

        // check that names and types for formals are valid
        for (FormalNode* formal : formals->get_formals()) {
            std::string name = formal->get_name();
            std::string type = formal->get_type();

            if (name == Strings::Self) {
                error_msg << "'self' cannot be the name of a formal parameter.";
                semant_error(error_msg.str(), formal->get_line_number());
            }

            if (type == Strings::Types::SelfType) {
                error_msg << "Formal parameter " << name << " cannot have type SELF_TYPE.";
                semant_error(error_msg.str(), formal->get_line_number());
            }
        }

        if (env.methods.exists(cls_name, method_name)) {           
            // we're overriding a method

            // first, verify that this method was defined in a parent class
            // rather than being a multiply defined method
            if (added_methods.find(method_name) != added_methods.end()) {
                error_msg << "Method " << method_name << " is multiply defined.";
                semant_error(error_msg.str(), method->get_line_number());
            }

            // find the method to be overridden
            MethodNode* original_method = env.methods.find(cls_name, method_name);
            FormalsNode* original_formals = original_method->get_formals();

            // make sure return types match exactly
            if (original_method->get_type() != method->get_type()) {
                error_msg << "Attempted to override method " << method_name
                          << " with a different return type.";
                semant_error(error_msg.str(), method->get_line_number());
            }

            // make sure the number of formals matches that of the overridden method
            if (formals->length() != original_formals->length()) {
                error_msg << "Incompatible number of formal parameters in redefined method " << method_name << ".";
                semant_error(error_msg.str(), method->get_line_number());
            }

            // make sure the types of the formals match those of the overridden method
            for (size_t i = 0; i < formals->length(); ++i) {
                FormalNode* formal = formals->get_formals()[i];
                FormalNode* original_formal = original_formals->get_formals()[i];

                std::string new_type = formal->get_type();
                std::string orig_type = original_formal->get_type();

                // the type must match exactly, not just conform
                if (new_type != orig_type) {
                    error_msg << "In redefined method " << method_name << ", parameter type "
                              << new_type << " is different from original type " << orig_type << ".";
                    semant_error(error_msg.str(), formal->get_line_number());
                }
            }
        }

        added_methods.insert(method->get_name());
        env.methods.set(cls_name, method);
    }
}

void build_method_env(TypeEnvironment& env) {
    // build a global method environment
    // this is used by dispatch classes to call methods of other classes
    for (auto it = classtable->clsmap.begin(); it != classtable->clsmap.end(); ++it) {
        ClassNode* cls = it->second;
        add_class_to_method_env(cls, cls->get_name(), env);
    }
}

void build_class_object_env(ClassNode* cls, TypeEnvironment& env) {
    if (cls->get_name() != Strings::Types::Object) {
        // classes inherit all features from their parents,
        // so this function is called recursively until we reach Object
        ClassNode* parent = classtable->clsmap[cls->get_base_class()];
        build_class_object_env(parent, env);
    }

    // in building the object environment for a class, we only
    // care about attributes. methods are handled by the method environment
    std::vector<AttributeNode*> attributes = cls->get_attributes();

    for (FeatureNode* attribute : attributes) {      
        std::string name = attribute->get_name();
        std::string declared_type = attribute->get_type();

        // check for multiply defined attributes
        // this also checks attributes in parent classes, which must
        // not be overridden either
        if (env.objects.probe(name)) {
            error_msg << "Attribute " << name << " is already defined in class "
                      << cls->get_name() << " or an inherited class.";
            semant_error(error_msg.str(), attribute->get_line_number());
        }

        if (name == Strings::Self) {
            error_msg << "'self' cannot be the name of an attribute.";
            semant_error(error_msg.str(), attribute->get_line_number());
        }

        env.objects.add_object(name, declared_type);
    }
}

// this method does not return anything;
// instead it performs type inference and 
// annotates the given abstract syntax tree
ClassTable* ProgramNode::analyze() {
    // build class table
    classtable = new ClassTable(get_classes());
    
    // build method environment from the classes,
    auto env = std::make_unique<TypeEnvironment>();
    build_method_env(*env);

    // typecheck each class separately
    for (ClassNode* cls : get_classes()) {
        cls->analyze(*env);
    }

    return classtable;
}

void ClassNode::analyze(TypeEnvironment& env) {
    // set up the class environment!
    // set the current class and add the attributes of 
    // the class to the object environment
    env.cls = this;
    env.objects.enter_scope();

    build_class_object_env(this, env);
    env.objects.add_object(Strings::Self, Strings::Types::SelfType);

    for (FeatureNode* feature : get_features()) {
        feature->analyze(env);
    }

    env.objects.exit_scope();
}

void AttributeNode::analyze(TypeEnvironment& env) {
    std::string declared_type = get_type();
    ExpressionNode* initializer = get_expr();

    std::string inferred_type = initializer->typecheck(env);
    std::string resolved_inferred_type = resolve(inferred_type, env);

    if (resolved_inferred_type != Strings::Types::NoType) {
        if (classtable->least_upper_bound(resolved_inferred_type, declared_type) != declared_type) {
            error_msg << "Inferred type of initialization expression "
                      << inferred_type << " does not match "
                      << "declared type " << declared_type << ".";
            semant_error(error_msg.str(), initializer->get_line_number());
        }

        initializer->set_checked_type(inferred_type);
    }
}

void MethodNode::analyze(TypeEnvironment& env) {
    std::string method_name = get_name();
    std::string return_type = get_type();
    FormalsNode* formals = get_formals();
    ExpressionNode* expression = get_expr();

    // verify that the return type exists
    if (return_type != Strings::Types::SelfType && !classtable->exists(return_type)) {
        error_msg << "Undefined return type " << return_type << " in method " << method_name << ".";
        semant_error(error_msg.str(), get_line_number());
    }

    env.objects.enter_scope();

    // a method has its formals available, so we add
    // these to the object environment first
    for (FormalNode* formal : formals->get_formals()) {
        std::string name = formal->get_name();
        std::string declared_type = formal->get_type();

        // formals can't have duplicate names
        // we use probe rather than lookup, because we only want to look in the current scope;
        // it is legal to have a duplicate variable name deeper in the symbol table stack
        if (env.objects.probe(name)) {
            error_msg << "Formal parameter " << name << " is multiply defined.";
            semant_error(error_msg.str(), formal->get_line_number());
        }

        env.objects.add_object(name, declared_type);
    }

    std::string inferred_type = expression->typecheck(env);
    std::string resolved_inferred_type = resolve(inferred_type, env);
    std::string resolved_return_type = resolve(return_type, env);

    // we handle the case where a method returns a SELF_TYPE separately
    // this is because while SELF_TYPE might resolve to the correct type, 
    // it has to actually be SELF_TYPE - otherwise, inherited classes can return
    // the parent class rather than an instance of the inherited class
    if (return_type == Strings::Types::SelfType && inferred_type != Strings::Types::SelfType
       || (classtable->least_upper_bound(resolved_return_type, resolved_inferred_type) != resolved_return_type)) {
        error_msg << "Inferred return type " << inferred_type << " of method " << method_name
                  << " does not conform to declared return type " << return_type << ".";
        semant_error(error_msg.str(), expression->get_line_number());
    }

    expression->set_checked_type(inferred_type);
    env.objects.exit_scope();
}

/* 
 *  The basic classes
 */

std::string NoExpressionNode::typecheck(TypeEnvironment& env) {
    return Strings::Types::NoType;
}

std::string IntNode::typecheck(TypeEnvironment& env) {
    return Strings::Types::Int;
}

std::string StringNode::typecheck(TypeEnvironment& env) {
    return Strings::Types::String;
}

std::string BoolNode::typecheck(TypeEnvironment& env) {
    return Strings::Types::Bool;
}

std::string IdentifierNode::typecheck(TypeEnvironment& env) {
    std::string name = get_name();

    // the special variable self always has type SELF_TYPE
    if (name == Strings::Self) {
        return Strings::Types::SelfType;
    }

    // lookup the variable in the scope
    std::string type = env.objects.lookup(name);

    // if the lookup fails, it means the variable is undefined
    if (type.empty()) {
        error_msg << "Undeclared identifier " << name << ".";
        semant_error(error_msg.str(), get_line_number());
    }

    return type;
}

std::string AssignmentNode::typecheck(TypeEnvironment& env) {
    std::string name = get_name();

    if (name == Strings::Self) {
        error_msg << "Cannot assign to 'self'.";
        semant_error(error_msg.str(), get_line_number());
    }

    std::string declared_type = env.objects.lookup(name);
    if (declared_type.empty()) {
        error_msg << "Target identifier has not been declared";
        semant_error(error_msg.str(), get_line_number());
    }

    ExpressionNode* expression = get_expr();
    std::string inferred_type = expression->typecheck(env);

    std::string resolved_declared_type = resolve(declared_type, env);
    std::string resolved_inferred_type = resolve(inferred_type, env);

    // the expression must conform to the declared type of the variable
    if (classtable->least_upper_bound(resolved_declared_type, resolved_inferred_type) != resolved_declared_type) {
        error_msg << "Type " << inferred_type << " of assigned expression does not conform "
                  << "to declared type " << declared_type << " of identifier " << name << ".";
        semant_error(error_msg.str(), get_line_number());
    }

    expression->set_checked_type(inferred_type);

    return inferred_type;
}

std::string NewNode::typecheck(TypeEnvironment& env) {
    std::string type = get_type();
    std::string resolved_type = resolve(type, env);

    if (classtable->clsmap.find(resolved_type) == classtable->clsmap.end()) {
        error_msg << "'new' keyword used with undefined type " << type;
        semant_error(error_msg.str(), get_line_number());
    }

    set_checked_type(type);

    return type;
}

std::string IsvoidNode::typecheck(TypeEnvironment& env) {
    // isvoid returns a bool regardless of the expression
    // we process the expression anyway, so its type can be annotated
    ExpressionNode* expr = get_expr();
    std::string inferred_type = expr->typecheck(env);

    expr->set_checked_type(inferred_type);
    return Strings::Types::Bool;
}

std::string NegNode::typecheck(TypeEnvironment& env) {
    ExpressionNode* expression = get_expr();
    std::string inferred_type = expression->typecheck(env);

    if (inferred_type != Strings::Types::Int) {
        error_msg << "Invalid type " << inferred_type << " for integer complement operation.";
        semant_error(error_msg.str(), expression->get_line_number());
    }

    expression->set_checked_type(Strings::Types::Int);
    return Strings::Types::Int;
}

std::string ComplementNode::typecheck(TypeEnvironment& env) {
    ExpressionNode* expression = get_expr();
    std::string inferred_type = expression->typecheck(env);

    if (inferred_type != Strings::Types::Bool) {
        error_msg << "Invalid type " << inferred_type << " for not operation.";
        semant_error(error_msg.str(), expression->get_line_number());
    }

    expression->set_checked_type(Strings::Types::Bool);
    return Strings::Types::Bool;
}

// arithmetic expressions are only defined for integers,
// so verify that the input expressions are integers

std::string PlusNode::typecheck(TypeEnvironment& env) {
    ExpressionNode* first = get_first();
    ExpressionNode* second = get_second();

    std::string first_type = first->typecheck(env);
    std::string second_type = second->typecheck(env);

    if (first_type != Strings::Types::Int || second_type != Strings::Types::Int) {
        error_msg << "non-Int arguments: " << first_type << " + " << second_type;
        semant_error(error_msg.str(), get_line_number());
    }

    first->set_checked_type(Strings::Types::Int);
    second->set_checked_type(Strings::Types::Int);

    return Strings::Types::Int;
}

std::string MinusNode::typecheck(TypeEnvironment& env) {
    ExpressionNode* first = get_first();
    ExpressionNode* second = get_second();

    std::string first_type = first->typecheck(env);
    std::string second_type = second->typecheck(env);

    if (first_type != Strings::Types::Int || second_type != Strings::Types::Int) {
        error_msg << "non-Int arguments: " << first_type << " + " << second_type;
        semant_error(error_msg.str(), get_line_number());
    }

    first->set_checked_type(Strings::Types::Int);
    second->set_checked_type(Strings::Types::Int);

    return Strings::Types::Int;
}

std::string MultiplicationNode::typecheck(TypeEnvironment& env) {
    ExpressionNode* first = get_first();
    ExpressionNode* second = get_second();

    std::string first_type = first->typecheck(env);
    std::string second_type = second->typecheck(env);

    if (first_type != Strings::Types::Int || second_type != Strings::Types::Int) {
        error_msg << "non-Int arguments: " << first_type << " + " << second_type;
        semant_error(error_msg.str(), get_line_number());
    }

    first->set_checked_type(Strings::Types::Int);
    second->set_checked_type(Strings::Types::Int);

    return Strings::Types::Int;
}

std::string DivisionNode::typecheck(TypeEnvironment& env) {
    ExpressionNode* first = get_first();
    ExpressionNode* second = get_second();

    std::string first_type = first->typecheck(env);
    std::string second_type = second->typecheck(env);

    if (first_type != Strings::Types::Int || second_type != Strings::Types::Int) {
        error_msg << "non-Int arguments: " << first_type << " + " << second_type;
        semant_error(error_msg.str(), get_line_number());
    }

    first->set_checked_type(Strings::Types::Int);
    second->set_checked_type(Strings::Types::Int);

    return Strings::Types::Int;
}

std::string LTNode::typecheck(TypeEnvironment& env) {
    ExpressionNode* first = get_first();
    ExpressionNode* second = get_second();

    std::string first_type = first->typecheck(env);
    std::string second_type = second->typecheck(env);

    if (first_type != Strings::Types::Int || second_type != Strings::Types::Int) {
        error_msg << "non-Int arguments: " << first_type << " + " << second_type;
        semant_error(error_msg.str(), get_line_number());
    }

    first->set_checked_type(Strings::Types::Int);
    second->set_checked_type(Strings::Types::Int);

    return Strings::Types::Bool;
}

std::string LTENode::typecheck(TypeEnvironment& env) {
    ExpressionNode* first = get_first();
    ExpressionNode* second = get_second();

    std::string first_type = first->typecheck(env);
    std::string second_type = second->typecheck(env);

    if (first_type != Strings::Types::Int || second_type != Strings::Types::Int) {
        error_msg << "non-Int arguments: " << first_type << " + " << second_type;
        semant_error(error_msg.str(), get_line_number());
    }

    first->set_checked_type(Strings::Types::Int);
    second->set_checked_type(Strings::Types::Int);

    return Strings::Types::Bool;
}

std::string EQNode::typecheck(TypeEnvironment& env) {
    ExpressionNode* first = get_first();
    ExpressionNode* second = get_second();

    std::string first_type = first->typecheck(env);
    std::string second_type = second->typecheck(env);

    // unlike lt and lte, eq is defined for all types
    // however, when comparing basic types, the two types must be the same
    if (   (first_type == Strings::Types::Int && second_type != Strings::Types::Int)
        || (first_type == Strings::Types::String && second_type != Strings::Types::String)
        || (first_type == Strings::Types::Bool && second_type != Strings::Types::Bool)
    ) {
        error_msg << "Illegal comparison with a basic type.";
        semant_error(error_msg.str(), get_line_number());
    }

    first->set_checked_type(first_type);
    second->set_checked_type(second_type);

    return Strings::Types::Bool;
}

std::string ConditionalNode::typecheck(TypeEnvironment& env) {
    ExpressionNode* predicate = get_predicate();
    ExpressionNode* then_expr = get_then();
    ExpressionNode* else_expr = get_else();

    std::string pred_type = predicate->typecheck(env);
    std::string then_type = then_expr->typecheck(env);
    std::string else_type = else_expr->typecheck(env);

    // the predicate of a conditional expression must evaluate to true or false
    if (pred_type != Strings::Types::Bool) {
        error_msg << "Conditional predicate must be Bool, not " << pred_type << ".";
        semant_error(error_msg.str(), predicate->get_line_number());
    }

    predicate->set_checked_type(Strings::Types::Bool);
    then_expr->set_checked_type(then_type);
    else_expr->set_checked_type(else_type);

    std::string resolved_then_type = resolve(then_type, env);
    std::string resolved_else_type = resolve(else_type, env);

    std::string lub = classtable->least_upper_bound(resolved_then_type, resolved_else_type);

    // special case: if both branches are SELF_TYPE, return SELF_TYPE
    if (then_type == Strings::Types::SelfType && else_type == Strings::Types::SelfType) {
        return Strings::Types::SelfType;
    }

    return lub;
}

std::string WhileNode::typecheck(TypeEnvironment& env) {
    ExpressionNode* predicate = get_predicate();
    ExpressionNode* body = get_body();

    std::string pred_type = predicate->typecheck(env);
    std::string body_type = body->typecheck(env);

    // the loop predicate must evaluate to either true or false
    if (pred_type != Strings::Types::Bool) {
        error_msg << "Loop condition does not have type Bool.";
        semant_error(error_msg.str(), predicate->get_line_number());
    }

    predicate->set_checked_type(Strings::Types::Bool);
    body->set_checked_type(body_type);

    return Strings::Types::Object;
}

std::string BlockNode::typecheck(TypeEnvironment& env) {
    std::vector<ExpressionNode*> body = get_expressions();

    // iterate over the expressions in the block
    // this is just for annotation purposes - only the last
    // expression of the block body is relevant for type-checking purposes
    for (ExpressionNode* expr : body) {
        std::string type = expr->typecheck(env);
        expr->set_checked_type(type);
    }

    // return the type of the last expression in the body
    std::string type = body.back()->get_checked_type();
    return type;
}

std::string CaseNode::typecheck(TypeEnvironment& env) {
    ExpressionNode* expression = get_target();
    std::vector<CaseBranchNode*> branches = get_branches();

    std::string type = expression->typecheck(env);
    expression->set_checked_type(type);

    // keep track of both the branch conditions and bodies
    // in the case expression
    std::vector<std::string> branch_type_declarations;
    std::vector<std::string> branch_types;

    for (CaseBranchNode* branch : branches) {
        std::string identifier = branch->get_name();
        std::string declared_type = branch->get_type();
        ExpressionNode* expression = branch->get_expr();

        // the identifier in the case condition is accessible
        // in the branch body, so add it to the scope
        env.objects.enter_scope();
        env.objects.add_object(identifier, declared_type);

        std::string inferred_branch_type = expression->typecheck(env);
        std::string resolved_branch_type = resolve(inferred_branch_type, env);

        expression->set_checked_type(inferred_branch_type);

        // check if we've already registered a branch condition
        for (size_t i = 0; i < branch_type_declarations.size(); ++i) {
            if (branch_type_declarations[i] == declared_type) {
                error_msg << "Duplicate branch " << declared_type << " in case statement.";
                semant_error(error_msg.str(), expression->get_line_number());
            }
        }

        branch_type_declarations.push_back(declared_type);
        branch_types.push_back(resolved_branch_type);

        env.objects.exit_scope();        
    }

    // special case: if all branches are SELF_TYPE, return SELF_TYPE
    bool all_self_type = std::all_of(branches.begin(), branches.end(), [](CaseBranchNode* branch) {
        return branch->get_expr()->get_checked_type() == Strings::Types::SelfType;
    });
    
    if (all_self_type) {
        return Strings::Types::SelfType;
    }

    // the type of a case statement is the LUB of the branch types
    std::string lub = classtable->least_upper_bound(branch_types);
    return lub;
}

std::string LetNode::typecheck(TypeEnvironment& env) {
    std::vector<LetInitializerNode*> initializers = get_initializers();
    ExpressionNode* body = get_body();

    // the let variables are only available within the let statement
    env.objects.enter_scope();

    for (LetInitializerNode* init : initializers) {
        std::string name = init->get_name();
        std::string declared_type = init->get_type();
        std::string resolved_declared_type = resolve(declared_type, env);
        
        ExpressionNode* init_expr = init->get_expr();
        std::string init_type = init_expr->typecheck(env);
        std::string resolved_init_type = resolve(init_type, env);

        if (name == Strings::Self) {
            error_msg << "'self' cannot be bound in a 'let' expression.";
            semant_error(error_msg.str(), init_expr->get_line_number());
        }

        // like in attributes, it is not required that let initializers 
        // have an initial value, so we accept _no_type
        if (resolved_init_type != Strings::Types::NoType) {
            if (classtable->least_upper_bound(resolved_init_type, resolved_declared_type) != resolved_declared_type) {
                error_msg << "Inferred type " << init_type << " of initialization of " << name
                          << " does not conform to identifier's declared type " << declared_type << ".";
                semant_error(error_msg.str(), init_expr->get_line_number());
            }
        }

        init_expr->set_checked_type(init_type);
        env.objects.add_object(init->get_name(), declared_type);
    }

    std::string type = body->typecheck(env);
    body->set_checked_type(type);

    for (LetInitializerNode* init : initializers) {
        init->set_checked_type(type);
    }

    env.objects.exit_scope();

    return type;
}

std::string LetInitializerNode::typecheck(TypeEnvironment& env) {
    // is never called, but we need to implement it
    // because LetInitializerNode inherits from ExpressionNode
    return "";
}

std::string DispatchNode::typecheck(TypeEnvironment& env) {
    std::string method_name = get_method_name();
    ExpressionNode* object = get_object();
    std::vector<ExpressionNode*> parameters = get_parameters();

    std::string object_class = object->typecheck(env);
    object->set_checked_type(object_class);

    std::string resolved_class = resolve(object_class, env);

    // verify that the called method actually exists in the method environment
    if (!env.methods.exists(resolved_class, method_name)) {
        error_msg << "Dispatch to undefined method " << method_name << ".";
        semant_error(error_msg.str(), object->get_line_number());
    }

    MethodNode* method = env.methods.find(resolved_class, method_name);
    std::vector<FormalNode*> formals = method->get_formals()->get_formals();

    // check that the number of argument matches the method signature
    if (parameters.size() != formals.size()) {
        error_msg << "Method " << method_name << " in class " << resolved_class 
                  << " takes " << formals.size()
                  << " argument(s), " << parameters.size() << " argument(s) provided.";
        semant_error(error_msg.str(), method->get_line_number());
    }

    // check that the types of the arguments match the method signature
    for (size_t i = 0; i < formals.size(); ++i) {
        ExpressionNode* parameter = parameters[i];
        FormalNode* formal = formals[i];

        std::string formal_type = formal->get_type();
        std::string parameter_type = parameter->typecheck(env);
        std::string resolved_parameter_type = resolve(parameter_type, env);

        if (classtable->least_upper_bound(formal_type, resolved_parameter_type) != formal_type) {
            error_msg << "In call of method " << method_name << ", type " << parameter_type
                      << " of parameter " << formal->get_name() << " does not conform to"
                      << " declared type " << formal_type << ".";
            semant_error(error_msg.str(), parameter->get_line_number());
        }

        parameter->set_checked_type(parameter_type);
    }
  
    std::string return_type = method->get_type();

    // here, we resolve differently, because SELF_TYPE
    // refers to the class to which the method belongs,
    // not the current env.cls
    std::string resolved_return_type;
    if (return_type == Strings::Types::SelfType) {
        resolved_return_type = object_class;
    } else {
        resolved_return_type = return_type;
    }

    return resolved_return_type;
}

std::string StaticDispatchNode::typecheck(TypeEnvironment& env) {
    std::string method_name = get_method_name();
    std::string static_type = get_static_type();
    ExpressionNode* object = get_object();
    std::vector<ExpressionNode*> parameters = get_parameters();

    std::string object_type = object->typecheck(env);
    object->set_checked_type(object_type);

    std::string resolved_static_type = resolve(static_type, env);
    std::string resolved_object_type = resolve(object_type, env);

    // verify that the type of the target object conforms to the static dispatch type
    if (classtable->least_upper_bound(resolved_object_type, resolved_static_type) != resolved_static_type) {
        error_msg << "Expression type " << object_type
                  << " does not conform to declared static dispatch type " << static_type << ".";
        semant_error(error_msg.str(), object->get_line_number());
    }

    // verify that the called method actually exists in the method environment
    if (!env.methods.exists(resolved_static_type, method_name)) {
        error_msg << "Dispatch to undefined method " << method_name << ".";
        semant_error(error_msg.str(), object->get_line_number());
    }

    MethodNode* method = env.methods.find(resolved_static_type, method_name);
    std::vector<FormalNode*> formals = method->get_formals()->get_formals();

    // check that the number of argument matches the method signature
    if (parameters.size() != formals.size()) {
        error_msg << "Method " << method_name << " in class " << resolved_static_type
                  << " takes " <<  formals.size() << "argument(s), "
                  << parameters.size() << " argument(s) provided.";
        semant_error(error_msg.str(), method->get_line_number());
    }

    // check that the types of the arguments match the method signature
    for (size_t i = 0; i < formals.size(); ++i) {
        ExpressionNode* parameter = parameters[i];
        FormalNode* formal = formals[i];

        std::string formal_type = formal->get_type();
        std::string parameter_type = parameter->typecheck(env);
        std::string resolved_parameter_type = resolve(parameter_type, env);

        if (classtable->least_upper_bound(formal_type, resolved_parameter_type) != formal_type) {
            error_msg << "Parameter " << i+1 << " of method " << method_name
                      << " in class " << resolved_static_type << " accepts expressions of type "
                      << formal_type << ", type " << parameter_type << " provided.";
            semant_error(error_msg.str(), parameter->get_line_number());
        }

        parameter->set_checked_type(parameter_type);
    }

    std::string return_type = method->get_type();
    std::string resolved_return_type;

    // like in DispatchNode, we resolve SELF_TYPE differently,
    // because as a return type, it refers to the class to which
    // the method belongs
    if (return_type == Strings::Types::SelfType) {
        resolved_return_type = object_type;
    } else {
        resolved_return_type = return_type;
    }

    return resolved_return_type;
}