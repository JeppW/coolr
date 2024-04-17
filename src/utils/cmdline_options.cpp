#include "cmdline_options.h"

/*
 *  Utility for parsing command line options.
 */

void CmdlineOptions::print_usage(int exit_code = 0) {
    std::cerr << "Usage: ./coolr <sourcefile> [options]\n";
    std::cerr << "Options:\n";
    std::cerr << "  --help\t\t\tPrint this help message\n";
    std::cerr << "  --out <file>\t\t\tSpecify the output file (default: out.S)\n";
    std::cerr << "  --lex\t\t\t\tStop after lexical analysis\n";
    std::cerr << "  --parse\t\t\tStop after parsing\n";
    std::cerr << "  --semant\t\t\tStop after semantic analysis\n";
    exit(exit_code);
}

CmdlineOptions::CmdlineOptions(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(1);
    }

    sourcefile = std::string(argv[1]);

    for (int i = 1; i < argc; ++i) {
        std::string arg = std::string(argv[i]);
        if (arg == "--help") {
            print_usage();
        } else if (arg == "--lex") {
            stop_after = StopAfter::LEX;
        } else if (arg == "--parse") {
            stop_after = StopAfter::PARSE;
        } else if (arg == "--semant") {
            stop_after = StopAfter::SEMANT;
        } else if (arg == "--out") { 
            if (argc > i + 1) {
                outfile = std::string(argv[++i]); 
            } else {
                throw std::runtime_error("Output file name not specified after --out.");
            }
        }
    }
};