#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include "utils/cmdline_options.h"
#include "common/classtable.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"
#include "compiler/semant/semant.h"
#include "compiler/codegen/codegen.h"

void compile(std::stringstream& program, CmdlineOptions* options) {
    Scanner scanner;
    Parser parser;

    Tokenstream ts = scanner.scan(program);
    if (options->get_stop_after() == StopAfter::LEX) {
        for (auto token : ts.get_tokens()) {
            token->dump();
        }
        return;
    }

    ProgramNode ast = parser.parse(ts);
    if (options->get_stop_after() == StopAfter::PARSE) {
        ast.dump(0);
        return;
    }

    ClassTable* classtable = ast.analyze();
    if (options->get_stop_after() == StopAfter::SEMANT) {
        ast.dump(0);
        return;
    }

    generate_code(ast, options->get_outfile_name(), classtable);
}

int main(int argc, char *argv[]) {
    CmdlineOptions* options = new CmdlineOptions(argc, argv);
    
    std::stringstream buffer;
    std::ifstream t_file(options->get_sourcefile_name());

    if (!t_file) {
        throw std::runtime_error("Invalid file.");
    }

    buffer << t_file.rdbuf();

    compile(buffer, options);

    return 0;
}