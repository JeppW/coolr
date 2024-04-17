#include "classtag.h"

/*
 *  Methods for generating and retrieving unique class tags.
 */

// start naming classes from 100
static uint class_tag_num = 100;
static std::map<std::string, uint> class_tag_map;

uint get_class_tag(const std::string& cls) {
    if (class_tag_map.find(cls) == class_tag_map.end()) {
        // new class tag
        class_tag_map[cls] = class_tag_num++;
    }

    return class_tag_map[cls];
}

std::string get_class_by_tag(uint tag) {
    for (const auto& pair : class_tag_map) {
        if (pair.second == tag) {
            return pair.first;
        }
    }

    throw std::runtime_error("Class tag not found");
}