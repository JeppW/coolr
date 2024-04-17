#ifndef CLASSTABLE_H
#define CLASSTABLE_H

#include <map>
#include <vector>
#include <sstream>
#include "ast.h"
#include "../common/consts.h"
#include "../utils/errors.h"

class ClassTable {
    private:
        void install_basic_classes();
        void check_inheritance_graph();

    public:
        ClassTable(std::vector<ClassNode*>);
        std::map<std::string, ClassNode*> clsmap;
        std::vector<std::string> get_ancestry(const std::string&);
        bool exists(const std::string&);
        std::string least_upper_bound(const std::string&, const std::string&);
        std::string least_upper_bound(std::vector<std::string>);
};

#endif