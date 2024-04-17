#include "pretty_print.h"

/*
 *  Utilities for escaping special characters in strings, used for formatting output.
 */

std::string get_escaped_string(const std::string& value) {
    std::ostringstream pretty_value;

    for (char c : value) {
        switch (c) {
            case '\n':
                pretty_value << "\\n";
                break;
            case '\r':
                pretty_value << "\\015";
                break;
            case '\x1b':
                pretty_value << "\\033";
                break;
            case '\t':
                pretty_value << "\\t";
                break;
            case '\b':
                pretty_value << "\\b";
                break;
            case '\f':
                pretty_value << "\\f";
                break;
            case '"':
                pretty_value << "\\\"";
                break;
            case '\\':
                pretty_value << "\\\\";
                break;
            default:
                if (std::isprint(c)) {
                    pretty_value << c;
                } else {
                    pretty_value << "\\" << std::setw(3) << std::setfill('0') << uint(c);
                }
                break;
        }
    }

    return pretty_value.str();
}

std::string get_pretty_string(const std::string& value) {
    // print so it makes the Stanford grading tests happy  
    return '"' + get_escaped_string(value) + '"';
}