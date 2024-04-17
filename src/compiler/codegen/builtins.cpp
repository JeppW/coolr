#include "builtins.h"

/*
 *  This file implements built-in features of the COOL languages as well 
 *  as internal routines use in the compiler in hand-written x86 assembly.
*/

// static error message strings for runtime errors
static const std::string abort_err_str = "Abort called from class '";
static const std::string dispatch_to_void_err_str = "Dispatch to void\\n";
static const std::string out_of_memory_err_str = "Out of memory\\n";
static const std::string index_out_of_bounds_err_str = "Index out of range\\n";
static const std::string match_on_void_err_str = "Match on void in case statement\\n";
static const std::string no_match_err_str = "No match in case statement\\n";

std::string code_uninitialized_basic_objects() {
    std::stringstream ss;

    ss << Asm::label(uninitialized_string);
    ss << Asm::dd(get_class_tag(Strings::Types::String));
    ss << Asm::dd("String_typename");
    ss << Asm::dd((Constants::NumObjHeaders + 2) * Constants::WordSize);
    ss << Asm::dd("String_dispatch_table");
    ss << Asm::dd("Object_proto");
    ss << Asm::dd(0);
    ss << Asm::dd(empty_string);
    ss << Asm::newline();

    ss << Asm::label(uninitialized_int);
    ss << Asm::dd(get_class_tag(Strings::Types::Int));
    ss << Asm::dd("Int_typename");
    ss << Asm::dd((Constants::NumObjHeaders + 1) * Constants::WordSize);
    ss << Asm::dd("Int_dispatch_table");
    ss << Asm::dd("Object_proto");
    ss << Asm::dd(0);
    ss << Asm::newline();

    ss << Asm::label(uninitialized_bool);
    ss << Asm::dd(get_class_tag(Strings::Types::Bool));
    ss << Asm::dd("Bool_typename");
    ss << Asm::dd((Constants::NumObjHeaders + 1) * Constants::WordSize);
    ss << Asm::dd("Bool_dispatch_table");
    ss << Asm::dd("Object_proto");
    ss << Asm::dd(0);
    ss << Asm::newline();

    return ss.str();
}

std::string code_heap() {
    std::stringstream ss;

    ss << Asm::dd(heapptr, heapstart);
    ss << Asm::label(heapstart);
    ss << Asm::empty_memory(10000000);
    ss << Asm::label(heapend);
    ss << Asm::newline();

    return ss.str();
}

std::string code_input_buffer() {
    std::stringstream ss;

    ss << Asm::label(inputbuffer);
    ss << Asm::empty_memory(Constants::MaxStringSize+1);
    ss << Asm::newline();

    return ss.str();
}

std::string code_builtin_methods() {
    std::stringstream ss;

    ss << Asm::label("Object.abort");
    ss << Asm::enter();
    ss << Asm::mov(eax, 4);
    ss << Asm::mov(ebx, 1);
    ss << Asm::mov(ecx, "_abort_error_msg");
    ss << Asm::mov(edx, 24);
    ss << Asm::syscall();
    ss << Asm::mov(eax, ptr(selfptr));
    ss << Asm::push(eax);
    ss << Asm::call("Object.type_name");    // retrieve and print class name
    ss << Asm::add(eax, get_attr_offset(Strings::Types::String, Strings::Attributes::StrField));
    ss << Asm::mov(eax, ptr(eax));
    ss << Asm::mov(ecx, eax);
    ss << Asm::push(ecx);
    ss << Asm::call("_strlen");
    ss << Asm::mov(edx, eax);
    ss << Asm::mov(eax, 4);
    ss << Asm::mov(ebx, 1);
    ss << Asm::syscall();
    ss << Asm::mov(eax, 4);
    ss << Asm::mov(ebx, 1);
    ss << Asm::push(10);                    // push and print newline character
    ss << Asm::mov(ecx, esp);
    ss << Asm::mov(edx, 1);
    ss << Asm::syscall();
    ss << Asm::jmp("_error_exit");          // exit with an error
    ss << Asm::newline();
    
    ss << Asm::label("Object.type_name");
    ss << Asm::enter();
    ss << Asm::mov(eax, ptr(selfptr));
    ss << Asm::add(eax, 4);
    ss << Asm::mov(eax, ptr(eax));
    ss << Asm::push(eax);
    ss << Asm::replace_selfptr("String_proto");
    ss << Asm::call("Object.copy");         // allocate new String object on heap
    ss << Asm::restore_selfptr();
    ss << Asm::add(eax, get_attr_offset(Strings::Types::String, Strings::Attributes::StrField));
    ss << Asm::pop(ebx);
    ss << Asm::mov(ptr(eax), ebx);          // copy class name to str_field of new String object
    ss << Asm::sub(eax, 4);
    ss << Asm::push(eax);
    ss << Asm::push(ebx);
    ss << Asm::call("_strlen");             // set length of string
    ss << Asm::pop(ebx);
    ss << Asm::mov(ptr(ebx), eax);
    ss << Asm::mov(eax, ebx);
    ss << Asm::sub(eax, Constants::NumObjHeaders * Constants::WordSize);
    ss << Asm::leave();
    ss << Asm::ret();
    ss << Asm::newline();

    ss << Asm::label("Object.copy");
    ss << Asm::enter();
    ss << Asm::mov(eax, ptr(selfptr));      // call _allocate_memory with the object size
    ss << Asm::add(eax, 8);                 // as parameter
    ss << Asm::mov(eax, ptr(eax));
    ss << Asm::push(eax);
    ss << Asm::push(eax);
    ss << Asm::call("_allocate_memory");
    ss << Asm::pop(ecx);
    ss << Asm::mov(edi, eax);
    ss << Asm::mov(esi, ptr(selfptr));
    ss << Asm::cld();                       // copy object to location returned 
    ss << Asm::rep_movsb();                 // by _allocate_memory
    ss << Asm::leave();
    ss << Asm::ret();
    ss << Asm::newline();

    ss << Asm::label("IO.out_string");
    ss << Asm::enter();
    ss << Asm::mov(ecx, ptr(ebp, 8));       // retrieve raw string from String parameter
    ss << Asm::add(ecx, get_attr_offset(Strings::Types::String, Strings::Attributes::StrField));
    ss << Asm::mov(ecx, ptr(ecx));          
    ss << Asm::push(ecx);
    ss << Asm::push(ecx);
    ss << Asm::call("_strlen");             // get length of string, used in syscall
    ss << Asm::mov(edx, eax);
    ss << Asm::pop(ecx);
    ss << Asm::mov(eax, 4);                 // syscall to write to stdout
    ss << Asm::mov(ebx, 1);
    ss << Asm::syscall();
    ss << Asm::mov(eax, ptr(selfptr));
    ss << Asm::leave();
    ss << Asm::ret(4);
    ss << Asm::newline();

    ss << Asm::label("IO.out_int");
    ss << Asm::enter();
    ss << Asm::mov(eax, ptr(ebp, 8));
    ss << Asm::add(eax, Constants::NumObjHeaders * Constants::WordSize);
    ss << Asm::mov(eax, ptr(eax));
    ss << Asm::test(eax, eax);
    ss << Asm::jns(".print_positive");
    ss << Asm::push(eax);
    ss << Asm::push(45);                    // if number is negative,
    ss << Asm::mov(ebx, 1);                 // push and print '-' character
    ss << Asm::lea(ecx, ptr(esp));
    ss << Asm::mov(edx, 1);
    ss << Asm::mov(eax, 4);
    ss << Asm::syscall();
    ss << Asm::add(esp, 4);
    ss << Asm::pop(eax);
    ss << Asm::neg(eax);                    // if negative, negate number
    ss << Asm::label(".print_positive");    // then, print number
    ss << Asm::call(".start");
    ss << Asm::leave();
    ss << Asm::ret(4);
    ss << Asm::label(".start");
    ss << Asm::push(eax);
    ss << Asm::push(edx);
    ss << Asm::xor_(edx, edx);
    ss << Asm::mov(ecx, 10);
    ss << Asm::div(ecx);
    ss << Asm::test(eax, eax);
    ss << Asm::je(".finish");
    ss << Asm::call(".start");
    ss << Asm::label(".finish");
    ss << Asm::lea(eax, ptr(edx, 0x30));
    ss << Asm::mov(ebx, 1);
    ss << Asm::push(eax);
    ss << Asm::lea(ecx, ptr(esp));
    ss << Asm::mov(edx, 1);
    ss << Asm::mov(eax, 4);
    ss << Asm::syscall();
    ss << Asm::add(esp, 4);
    ss << Asm::pop(edx);
    ss << Asm::pop(eax);
    ss << Asm::ret();
    ss << Asm::newline();

    ss << Asm::label("IO.in_string");
    ss << Asm::enter();
    ss << Asm::mov(eax, 3);                 // get string from stdin
    ss << Asm::mov(ebx, 0);
    ss << Asm::mov(ecx, inputbuffer);
    ss << Asm::mov(edx, Constants::MaxStringSize);
    ss << Asm::syscall();
    ss << Asm::xor_(eax, eax);
    ss << Asm::mov(edi, inputbuffer);
    ss << Asm::label(".loop");
    ss << Asm::cmp(byte_ptr(edi), 10);      // look for newline
    ss << Asm::je(".done");
    ss << Asm::inc(edi);
    ss << Asm::inc(eax);
    ss << Asm::jmp(".loop");
    ss << Asm::label(".done");
    ss << Asm::push(eax);                   // allocate string on heap
    ss << Asm::inc(eax);
    ss << Asm::push(eax);
    ss << Asm::call("_allocate_memory");
    ss << Asm::mov(edi, eax);
    ss << Asm::mov(esi, inputbuffer);
    ss << Asm::pop(ecx);
    ss << Asm::push(edi);
    ss << Asm::push(ecx);
    ss << Asm::cld();
    ss << Asm::rep_movsb();
    ss << Asm::mov(byte_ptr(edi), 0);            // terminating null byte
    ss << Asm::replace_selfptr("String_proto");  // allocate new String object on heap
    ss << Asm::call("Object.copy");
    ss << Asm::restore_selfptr();
    ss << Asm::mov(edx, eax);
    ss << Asm::add(eax, get_attr_offset(Strings::Types::String, Strings::Attributes::Val));
    ss << Asm::pop(ebx);
    ss << Asm::mov(ptr(eax), ebx);
    ss << Asm::add(eax, get_attr_offset(Strings::Types::String, Strings::Attributes::StrField) 
                        - get_attr_offset(Strings::Types::String, Strings::Attributes::Val));
    ss << Asm::pop(ebx);
    ss << Asm::mov(ptr(eax), ebx);
    ss << Asm::mov(eax, edx);
    ss << Asm::leave();
    ss << Asm::ret();
    ss << Asm::newline();

    ss << Asm::label("IO.in_int");
    ss << Asm::enter();
    ss << Asm::call("IO.in_string");        // get string from stdin using the in_string method
    ss << Asm::mov(edi, ptr(eax, get_attr_offset(Strings::Types::String, Strings::Attributes::StrField)));
    ss << Asm::mov(ebx, ptr(eax, get_attr_offset(Strings::Types::String, Strings::Attributes::Val)));
    ss << Asm::add(edi, ebx);
    ss << Asm::dec(edi);
    ss << Asm::xor_(ecx, ecx);
    ss << Asm::mov(edx, 1);
    ss << Asm::label(".loop");              // convert string to integer
    ss << Asm::test(ebx, ebx);
    ss << Asm::je(".done");
    ss << Asm::movzx(eax, byte_ptr(edi));
    ss << Asm::sub(eax, 0x30);
    ss << Asm::push(edx);
    ss << Asm::mul(edx);
    ss << Asm::pop(edx);
    ss << Asm::add(ecx, eax);
    ss << Asm::dec(edi);
    ss << Asm::dec(ebx);
    ss << Asm::mov(eax, edx);
    ss << Asm::mov(edx, 10);
    ss << Asm::mul(edx);
    ss << Asm::mov(edx, eax);
    ss << Asm::jmp(".loop");
    ss << Asm::label(".done");
    ss << Asm::push(ecx);                   // allocate new Int object on heap
    ss << Asm::replace_selfptr("Int_proto");
    ss << Asm::call("Object.copy");
    ss << Asm::restore_selfptr();
    ss << Asm::mov(edx, eax);
    ss << Asm::add(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val));
    ss << Asm::pop(ebx);
    ss << Asm::mov(ptr(eax), ebx);          // copy result to val attribute
    ss << Asm::mov(eax, edx);
    ss << Asm::leave();
    ss << Asm::ret();
    ss << Asm::newline();

    ss << Asm::label("String.length");
    ss << Asm::enter();                     // access the val attribute
    ss << Asm::mov(eax, ptr(selfptr));      // containing the string length
    ss << Asm::add(eax, get_attr_offset(Strings::Types::String, Strings::Attributes::Val));
    ss << Asm::mov(eax, ptr(eax));
    ss << Asm::push(eax);
    ss << Asm::replace_selfptr("Int_proto"); 
    ss << Asm::call("Object.copy");
    ss << Asm::restore_selfptr();
    ss << Asm::mov(edx, eax);
    ss << Asm::add(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val));
    ss << Asm::pop(ebx);
    ss << Asm::mov(ptr(eax), ebx);          // allocate new Int and
    ss << Asm::mov(eax, edx);               // copy length to val attribute
    ss << Asm::leave();
    ss << Asm::ret();
    ss << Asm::newline();

    ss << Asm::label("String.concat");
    ss << Asm::enter();
    ss << Asm::call("String.length");       // get length of first string
    ss << Asm::add(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val));
    ss << Asm::mov(eax, ptr(eax));
    ss << Asm::push(eax);
    ss << Asm::mov(edi, ptr(ebp, 8));
    ss << Asm::mov(ecx, ptr(selfptr));
    ss << Asm::push(ecx);
    ss << Asm::mov(dword_ptr(selfptr), edi);
    ss << Asm::call("String.length");       // get length of second string
    ss << Asm::add(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val));
    ss << Asm::mov(eax, ptr(eax));
    ss << Asm::pop(ecx);
    ss << Asm::mov(dword_ptr(selfptr), ecx);
    ss << Asm::push(eax);
    ss << Asm::mov(eax, ptr(ebp, -4));
    ss << Asm::mov(ebx, ptr(ebp, -8));
    ss << Asm::add(eax, ebx);               
    ss << Asm::push(eax);                   // add the lengths of the two strings
    ss << Asm::inc(eax);                    // plus one for terminating null byte
    ss << Asm::push(eax);                   // and allocate memory of that size
    ss << Asm::call("_allocate_memory");    
    ss << Asm::mov(edi, eax);
    ss << Asm::mov(esi, ptr(selfptr));
    ss << Asm::add(esi, get_attr_offset(Strings::Types::String, Strings::Attributes::StrField));
    ss << Asm::mov(esi, ptr(esi));
    ss << Asm::mov(ecx, ptr(ebp, -4));
    ss << Asm::cld();                       // copy first string to new location
    ss << Asm::rep_movsb();
    ss << Asm::mov(esi, ptr(ebp, 8));
    ss << Asm::add(esi, get_attr_offset(Strings::Types::String, Strings::Attributes::StrField));
    ss << Asm::mov(esi, ptr(esi));
    ss << Asm::mov(ecx, ptr(ebp, -8));
    ss << Asm::inc(ecx);
    ss << Asm::cld();                       // copy second string to new location
    ss << Asm::rep_movsb();
    ss << Asm::push(eax);
    ss << Asm::replace_selfptr("String_proto");
    ss << Asm::call("Object.copy");
    ss << Asm::restore_selfptr();
    ss << Asm::mov(ebx, eax);               // make and return new String object
    ss << Asm::add(eax, get_attr_offset(Strings::Types::String, Strings::Attributes::StrField));
    ss << Asm::pop(ecx);
    ss << Asm::mov(ptr(eax), ecx);
    ss << Asm::sub(eax, 4);
    ss << Asm::pop(ecx);
    ss << Asm::mov(ptr(eax), ecx);
    ss << Asm::mov(eax, ebx);
    ss << Asm::leave();
    ss << Asm::ret(4);
    ss << Asm::newline();

    ss << Asm::label("String.substr");
    ss << Asm::enter();
    ss << Asm::mov(eax, ptr(ebp, 12));      // get start index
    ss << Asm::add(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val));
    ss << Asm::mov(eax, ptr(eax));
    ss << Asm::cmp(eax, 0);                 // verify that it is in bounds (>= 0)
    ss << Asm::jl(".error");
    ss << Asm::mov(ebx, ptr(ebp, 8));       // get end index and
    ss << Asm::add(ebx, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val));
    ss << Asm::mov(ebx, ptr(ebx));          
    ss << Asm::add(ebx, eax);
    ss << Asm::push(ebx);                   
    ss << Asm::call("String.length");       // get length of string
    ss << Asm::add(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val));
    ss << Asm::mov(eax, ptr(eax));
    ss << Asm::pop(ebx);
    ss << Asm::cmp(ebx, eax);
    ss << Asm::jg(".error");                // verify that end index is in bounds
    ss << Asm::mov(eax, ptr(ebp, 8));
    ss << Asm::add(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val));
    ss << Asm::mov(eax, ptr(eax));
    ss << Asm::inc(eax);
    ss << Asm::push(eax);
    ss << Asm::call("_allocate_memory");    // allocate memory for new string
    ss << Asm::mov(edi, eax);
    ss << Asm::mov(ecx, ptr(ebp, 8));
    ss << Asm::add(ecx, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val));
    ss << Asm::mov(ecx, ptr(ecx));
    ss << Asm::mov(esi, ptr(selfptr));
    ss << Asm::add(esi, get_attr_offset(Strings::Types::String, Strings::Attributes::StrField));
    ss << Asm::mov(esi, ptr(esi));
    ss << Asm::mov(eax, ptr(ebp, 12));
    ss << Asm::add(eax, get_attr_offset(Strings::Types::Int, Strings::Attributes::Val));
    ss << Asm::mov(eax, ptr(eax));
    ss << Asm::add(esi, eax);
    ss << Asm::push(edi);
    ss << Asm::push(ecx);
    ss << Asm::cld();                       // copy new string to new location
    ss << Asm::rep_movsb();
    ss << Asm::mov(byte_ptr(edi), 0);
    ss << Asm::pop(ebx);
    ss << Asm::pop(eax);
    ss << Asm::jmp(".done");
    ss << Asm::label(".error");             // error handler
    ss << Asm::jmp("_index_out_of_bounds");
    ss << Asm::label(".done");
    ss << Asm::push(eax);
    ss << Asm::push(ebx);
    ss << Asm::replace_selfptr("String_proto");
    ss << Asm::call("Object.copy");
    ss << Asm::restore_selfptr();
    ss << Asm::mov(edx, eax);               // make and return new String object
    ss << Asm::pop(ebx);
    ss << Asm::add(eax, get_attr_offset(Strings::Types::String, Strings::Attributes::Val));
    ss << Asm::mov(ptr(eax), ebx);
    ss << Asm::pop(ebx);
    ss << Asm::add(eax, 4);
    ss << Asm::mov(ptr(eax), ebx);
    ss << Asm::mov(eax, edx);
    ss << Asm::leave();
    ss << Asm::ret(8);
    ss << Asm::newline();

    return ss.str();
}

std::string code_builtin_static_strings() {
    std::stringstream ss;

    ss << Asm::static_string(empty_string, "");
    ss << Asm::newline();

    ss << Asm::comment("error messages");
    ss << Asm::static_string("_abort_error_msg", abort_err_str);
    ss << Asm::static_string("_dispatch_to_void_msg", dispatch_to_void_err_str);
    ss << Asm::static_string("_out_of_memory_msg", out_of_memory_err_str);
    ss << Asm::static_string("_index_out_of_bounds_msg", index_out_of_bounds_err_str);
    ss << Asm::static_string("_match_on_void_msg", match_on_void_err_str);
    ss << Asm::static_string("_no_match_msg", no_match_err_str);
    ss << Asm::newline();

    return ss.str();
}

std::string code_error_procedures() {
    // built-in procedures for run-time error handling
    std::stringstream ss;

    ss << Asm::label("_error_exit");
    ss << Asm::mov(eax, 1);        // call exit with error code 1
    ss << Asm::mov(ebx, 1);
    ss << Asm::syscall();
    ss << Asm::newline();

    ss << Asm::label("_dispatch_to_void");
    ss << Asm::mov(eax, 4);
    ss << Asm::mov(ebx, 1);
    ss << Asm::mov(ecx, "_dispatch_to_void_msg");
    ss << Asm::mov(edx, dispatch_to_void_err_str.length() - 1);
    ss << Asm::syscall();
    ss << Asm::jmp("_error_exit");
    ss << Asm::newline();

    ss << Asm::label("_out_of_memory");
    ss << Asm::mov(eax, 4);
    ss << Asm::mov(ebx, 1);
    ss << Asm::mov(ecx, "_out_of_memory_msg");
    ss << Asm::mov(edx, out_of_memory_err_str.length() - 1);
    ss << Asm::syscall();
    ss << Asm::jmp("_error_exit");
    ss << Asm::newline();

    ss << Asm::label("_index_out_of_bounds");
    ss << Asm::mov(eax, 4);
    ss << Asm::mov(ebx, 1);
    ss << Asm::mov(ecx, "_index_out_of_bounds_msg");
    ss << Asm::mov(edx, index_out_of_bounds_err_str.length() - 1);
    ss << Asm::syscall();
    ss << Asm::jmp("_error_exit");
    ss << Asm::newline();

    ss << Asm::label("_match_on_void");
    ss << Asm::mov(eax, 4);
    ss << Asm::mov(ebx, 1);
    ss << Asm::mov(ecx, "_match_on_void_msg");
    ss << Asm::mov(edx, match_on_void_err_str.length() - 1);
    ss << Asm::syscall();
    ss << Asm::jmp("_error_exit");
    ss << Asm::newline();

    ss << Asm::label("_no_match");
    ss << Asm::mov(eax, 4);
    ss << Asm::mov(ebx, 1);
    ss << Asm::mov(ecx, "_no_match_msg");
    ss << Asm::mov(edx, no_match_err_str.length() - 1);
    ss << Asm::syscall();
    ss << Asm::jmp("_error_exit");
    ss << Asm::newline();

    return ss.str();
}

std::string code_entrypoint() {
    std::stringstream ss;

    // initialize Main class and call main method
    ss << Asm::label("_start");
    ss << Asm::enter();
    ss << Asm::call("Main._init");   
    ss << Asm::mov(ptr(selfptr), eax);
    ss << Asm::call("Main.main");
    ss << Asm::jmp("_exit");   // once done, exit with success
    ss << Asm::newline();

    // exit with error code 0
    ss << Asm::label("_exit");
    ss << Asm::mov(eax, 1);
    ss << Asm::mov(ebx, 0);
    ss << Asm::syscall();
    ss << Asm::newline();

    return ss.str();
}

std::string code_internal_routines() {
    // various routines used internally
    std::stringstream ss;

    // get the length of a null-terminated string
    ss << Asm::label("_strlen");
    ss << Asm::enter();
    ss << Asm::xor_(eax, eax);
    ss << Asm::mov(edi, ptr(ebp, 8));
    ss << Asm::label(".loop");
    ss << Asm::cmp(byte_ptr(edi), 0);
    ss << Asm::je(".done");
    ss << Asm::inc(edi);
    ss << Asm::inc(eax);
    ss << Asm::jmp(".loop");
    ss << Asm::label(".done");
    ss << Asm::leave();
    ss << Asm::ret(4);
    ss << Asm::newline();

    // compare two null-terminated strings
    ss << Asm::label("_strcmp");
    ss << Asm::enter();
    ss << Asm::mov(eax, ptr(ebp, 8));
    ss << Asm::mov(ebx, ptr(ebp, 12));
    ss << Asm::label(".loopstart");
    ss << Asm::movzx(ecx, byte_ptr(eax));
    ss << Asm::movzx(edx, byte_ptr(ebx));
    ss << Asm::cmp(ecx, edx);
    ss << Asm::jne(".notequal");
    ss << Asm::test(ecx, ecx);
    ss << Asm::je(".equal");
    ss << Asm::inc(eax);
    ss << Asm::inc(ebx);
    ss << Asm::jmp(".loopstart");
    ss << Asm::label(".equal");
    ss << Asm::replace_selfptr("Bool_proto");
    ss << Asm::call("Object.copy");
    ss << Asm::restore_selfptr();
    ss << Asm::mov(edx, eax);
    ss << Asm::add(eax, get_attr_offset(Strings::Types::Bool, Strings::Attributes::Val));
    ss << Asm::mov(dword_ptr(eax), 1);
    ss << Asm::mov(eax, edx);
    ss << Asm::jmp(".done");
    ss << Asm::label(".notequal");
    ss << Asm::replace_selfptr("Bool_proto");
    ss << Asm::call("Object.copy");
    ss << Asm::restore_selfptr();
    ss << Asm::mov(edx, eax);
    ss << Asm::add(eax, get_attr_offset(Strings::Types::Bool, Strings::Attributes::Val));
    ss << Asm::mov(dword_ptr(eax), 0);
    ss << Asm::mov(eax, edx);
    ss << Asm::label(".done");
    ss << Asm::leave();
    ss << Asm::ret(8);
    ss << Asm::newline();

    // allocate memory from the heap
    ss << Asm::label("_allocate_memory");
    ss << Asm::enter();
    ss << Asm::mov(eax, ptr(heapptr));
    ss << Asm::mov(ebx, heapend);
    ss << Asm::mov(ecx, eax);
    ss << Asm::add(ecx, ptr(ebp, 8));
    ss << Asm::cmp(ecx, ebx);
    ss << Asm::jg(".failed");
    ss << Asm::mov(ptr(heapptr), ecx);
    ss << Asm::leave();
    ss << Asm::ret(4);
    ss << Asm::label(".failed");
    ss << Asm::jmp("_out_of_memory");
    ss << Asm::newline();

    return ss.str();
}