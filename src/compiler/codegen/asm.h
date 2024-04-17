#ifndef ASM_H
#define ASM_H

#include <string>
#include <sstream>
#include <cmath>
#include "../../common/consts.h"

static std::string eax = "eax";
static std::string ebx = "ebx";
static std::string ecx = "ecx";
static std::string edx = "edx";
static std::string edi = "edi";
static std::string esi = "esi";
static std::string ebp = "ebp";
static std::string esp = "esp";

static std::string al = "al";
static std::string ah = "ah";
static std::string bl = "bl";
static std::string bh = "bh";
static std::string cl = "cl";
static std::string ch = "ch";
static std::string dl = "dl";
static std::string dh = "dh";

static std::string selfptr = "selfptr";
static std::string heapptr = "heapptr";
static std::string heapstart = "heapstart";
static std::string heapend = "heapend";
static std::string inputbuffer = "inputbuffer";

static std::string uninitialized_string = "uninitialized_string";
static std::string uninitialized_int = "uninitialized_int";
static std::string uninitialized_bool = "uninitialized_bool";

static std::string empty_string = "empty_string";

std::string ptr(const std::string&);
std::string ptr(const std::string&, int);
std::string byte_ptr(const std::string&);
std::string byte_ptr(const std::string&, int);
std::string word_ptr(const std::string&);
std::string word_ptr(const std::string&, int);
std::string dword_ptr(const std::string&);
std::string dword_ptr(const std::string&, int);

namespace Asm {
    std::string data_section_start();
    std::string dd(const std::string&, const std::string&);
    std::string dd(const std::string&);
    std::string dd(uint);
    std::string empty_memory(uint);
    std::string static_string(const std::string&, const std::string&);

    std::string text_section_start();
    std::string enter();
    std::string leave();
    std::string ret();
    std::string ret(uint);
    std::string push(const std::string&);
    std::string push(uint);
    std::string pop(const std::string&);
    std::string mov(const std::string&, const std::string&);
    std::string mov(const std::string&, uint);
    std::string movzx(const std::string&, const std::string&);
    std::string lea(const std::string&, const std::string&);
    std::string xchg(const std::string&, const std::string&);
    std::string add(const std::string&, const std::string&);
    std::string add(const std::string&, uint);
    std::string sub(const std::string&, const std::string&);
    std::string sub(const std::string&, uint);
    std::string mul(const std::string&);
    std::string imul(const std::string&);
    std::string div(const std::string&);
    std::string xor_(const std::string&, const std::string&);
    std::string xor_(const std::string&, int);
    std::string neg(const std::string&);
    std::string inc(const std::string&);
    std::string dec(const std::string&);
    std::string cmp(const std::string&, const std::string&);
    std::string cmp(const std::string&, uint);
    std::string test(const std::string&, const std::string&);
    std::string setz(const std::string&);
    std::string setg(const std::string&);
    std::string setge(const std::string&);
    std::string jmp(const std::string&);
    std::string je(const std::string&);
    std::string jne(const std::string&);
    std::string jg(const std::string&);
    std::string jl(const std::string&);
    std::string jns(const std::string&);
    std::string call(const std::string&);
    std::string syscall();
    std::string cld();
    std::string rep_movsb();

    std::string label(const std::string&);
    std::string label(const std::string&, int);
    std::string comment(const std::string&);
    std::string comment(const std::string&, bool);
    std::string global(const std::string&);
    std::string newline();

    std::string replace_selfptr(const std::string&);
    std::string restore_selfptr();
}

#endif
