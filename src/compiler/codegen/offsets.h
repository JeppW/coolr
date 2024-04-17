#ifndef OFFSETS_H
#define OFFSETS_H

#include <map>
#include <string>

void set_attr_offset(const std::string&, const std::string&, uint);
uint get_attr_offset(const std::string&, const std::string&);

void set_method_offset(const std::string&, const std::string&, uint);
uint get_method_offset(const std::string&, const std::string&);

#endif