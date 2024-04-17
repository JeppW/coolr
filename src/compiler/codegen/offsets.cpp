#include "offsets.h"

/*
 *  Module for storing and retrieving offsets for attributes and methods.
 */

static std::map<std::pair<std::string, std::string>, uint> attr_offsets;
static std::map<std::pair<std::string, std::string>, uint> method_offsets;

void set_attr_offset(const std::string& cls, const std::string& attribute, uint offset) {
    attr_offsets[std::make_pair(cls, attribute)] = offset;
}

uint get_attr_offset(const std::string& cls, const std::string& attribute) {
    return attr_offsets[std::make_pair(cls, attribute)];
}

void set_method_offset(const std::string& cls, const std::string& method, uint offset) {
    attr_offsets[std::make_pair(cls, method)] = offset;
}

uint get_method_offset(const std::string& cls, const std::string& method) {
    return attr_offsets[std::make_pair(cls, method)];
}