#include "asm.h"

/*
 *  Collection of useful methods for generating assembly code.
 */

static const std::string INDENT = "  ";

std::string ptr(const std::string& a) {
    return "[" + a + "]";
}

std::string ptr(const std::string& a, int offset) {
    if (offset >= 0) {
        return "[" + a + "+" + std::to_string(offset) + "]";
    } else {
        return "[" + a + "-" + std::to_string(abs(offset)) + "]";
    }
}

std::string byte_ptr(const std::string& a) {
    return "BYTE [" + a + "]";
}

std::string byte_ptr(const std::string& a, int offset) {
    if (offset >= 0) {
        return "BYTE [" + a + "+" + std::to_string(offset) + "]";
    } else {
        return "BYTE [" + a + "-" + std::to_string(abs(offset)) + "]";
    }
}

std::string word_ptr(const std::string& a) {
    return "WORD [" + a + "]";
}

std::string word_ptr(const std::string& a, int offset) {
    if (offset >= 0) {
        return "WORD [" + a + "+" + std::to_string(offset) + "]";
    } else {
        return "WORD [" + a + "-" + std::to_string(abs(offset)) + "]";
    }
}

std::string dword_ptr(const std::string& a) {
    return "DWORD [" + a + "]";
}

std::string dword_ptr(const std::string& a, int offset) {
    if (offset >= 0) {
        return "DWORD [" + a + "+" + std::to_string(offset) + "]";
    } else {
        return "DWORD [" + a + "-" + std::to_string(abs(offset)) + "]";
    }
}

// helper for temporarily changing the value of the selfptr
std::string Asm::replace_selfptr(const std::string& tmp_val) {
    std::stringstream ss;

    ss << Asm::mov(ecx, ptr(selfptr));
    ss << Asm::push(ecx);
    ss << Asm::mov(dword_ptr(selfptr), tmp_val);

    return ss.str();
}

std::string Asm::restore_selfptr() {
    std::stringstream ss;

    ss << Asm::pop(ecx);
    ss << Asm::mov(dword_ptr(selfptr), ecx);

    return ss.str();
}

std::string Asm::data_section_start() {
    return "section .data\n";
}

std::string Asm::dd(const std::string& label, const std::string& value) {
    return INDENT + label + " dd " + value + "\n";
}

std::string Asm::dd(const std::string& value) {
    return INDENT + "dd " + value + "\n";
}

std::string Asm::dd(uint value) {
    return INDENT + "dd " + std::to_string(value) + "\n";
}

std::string Asm::static_string(const std::string& label, const std::string& value) {
    return INDENT + label + " db `" + value + "`, 0" + "\n";
}

std::string Asm::empty_memory(uint size) {
    return INDENT + "times " + std::to_string(size) + " db 0" + "\n";
}

std::string Asm::text_section_start() {
    return "section .text\n";
}

std::string Asm::enter() {
    return INDENT + "enter 0, 0\n"; 
}

std::string Asm::leave() {
    return INDENT + "leave\n";  
}

std::string Asm::ret() {
    return INDENT + "ret\n";
}

std::string Asm::ret(uint num) {
    return INDENT + "ret " + std::to_string(num) + "\n";
}

std::string Asm::push(const std::string& a) {
    return INDENT + "push " + a + "\n";
}

std::string Asm::push(uint a) {
    return INDENT + "push " + std::to_string(a) + "\n";
}

std::string Asm::pop(const std::string& a) {
    return INDENT + "pop " + a + "\n";
}

std::string Asm::mov(const std::string& a, const std::string& b) {
    return INDENT + "mov " + a + ", " + b + "\n";
}

std::string Asm::mov(const std::string& a, uint b) {
    return INDENT + "mov " + a + ", " + std::to_string(b) + "\n";
}

std::string Asm::lea(const std::string& a, const std::string& b) {
    return INDENT + "lea " + a + ", " + b + "\n";
}

std::string Asm::xchg(const std::string& a, const std::string& b) {
    return INDENT + "xchg " + a + ", " + b + "\n";
}

std::string Asm::movzx(const std::string& a, const std::string& b) {
    return INDENT + "movzx " + a + ", " + b + "\n";
}

std::string Asm::add(const std::string& a, const std::string& b) {
    return INDENT + "add " + a + ", " + b + "\n";
}

std::string Asm::add(const std::string& a, uint b) {
    return INDENT + "add " + a + ", " + std::to_string(b) + "\n";
}

std::string Asm::sub(const std::string& a, const std::string& b) {
    return INDENT + "sub " + a + ", " + b + "\n";
}

std::string Asm::sub(const std::string& a, uint b) {
    return INDENT + "sub " + a + ", " + std::to_string(b) + "\n";
}

std::string Asm::mul(const std::string& a) {
    return INDENT + "mul " + a + "\n";
}

std::string Asm::imul(const std::string& a) {
    return INDENT + "imul " + a + "\n";
}

std::string Asm::div(const std::string& a) {
    return INDENT + "div " + a + "\n";
}

std::string Asm::xor_(const std::string& a, const std::string& b) {
    return INDENT + "xor " + a + ", " + b + "\n";
}

std::string Asm::xor_(const std::string& a, int b) {
    return INDENT + "xor " + a + ", " + std::to_string(b) + "\n";
}

std::string Asm::neg(const std::string& a) {
    return INDENT + "neg " + a + "\n";
}

std::string Asm::inc(const std::string& a) {
    return INDENT + "inc " + a + "\n";
}

std::string Asm::dec(const std::string& a) {
    return INDENT + "dec " + a + "\n";
}

std::string Asm::cmp(const std::string& a, const std::string& b) {
    return INDENT + "cmp " + a + ", " + b + "\n";
}

std::string Asm::cmp(const std::string& a, uint b) {
    return INDENT + "cmp " + a + ", " + std::to_string(b) + "\n";
}

std::string Asm::test(const std::string& a, const std::string& b) {
    return INDENT + "test " + a + ", " + b + "\n";
}

std::string Asm::setz(const std::string& a) {
    return INDENT + "setz " + a + "\n";
}

std::string Asm::setg(const std::string& a) {
    return INDENT + "setg " + a + "\n";
}

std::string Asm::setge(const std::string& a) {
    return INDENT + "setge " + a + "\n";
}

std::string Asm::jmp(const std::string& a) {
    return INDENT + "jmp " + a + "\n";
}

std::string Asm::je(const std::string& a) {
    return INDENT + "je " + a + "\n";
}

std::string Asm::jne(const std::string& a) {
    return INDENT + "jne " + a + "\n";
}

std::string Asm::jg(const std::string& a) {
    return INDENT + "jg " + a + "\n";
}

std::string Asm::jl(const std::string& a) {
    return INDENT + "jl " + a + "\n";
}

std::string Asm::jns(const std::string& a) {
    return INDENT + "jns " + a + "\n";
}

std::string Asm::call(const std::string& a) {
    return INDENT + "call " + a + "\n";
}

std::string Asm::syscall() {
    return INDENT + "int 0x80\n";
}

std::string Asm::cld() {
    return INDENT + "cld\n";
}

std::string Asm::rep_movsb() {
    return INDENT + "rep movsb\n";
}

std::string Asm::label(const std::string& label) {
    return label + ":" + "\n";
}

std::string Asm::label(const std::string& label, int num) {
    return label + std::to_string(num) + ":" + "\n";
}

std::string Asm::comment(const std::string& comment) {
    return "; " + comment + "\n";
}

std::string Asm::comment(const std::string& comment, bool indent) {
    if (indent) {
        return INDENT + "; " + comment + "\n";
    } else {
        return "; " + comment + "\n";
    }
}

std::string Asm::global(const std::string& label) {
    return "global " + label + "\n";
}

std::string Asm::newline() {
    return "\n";
}
