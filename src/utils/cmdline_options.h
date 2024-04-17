#ifndef CMDLINE_OPTIONS_H
#define CMDLINE_OPTIONS_H

#include <string>
#include <stdexcept>
#include <iostream>

enum StopAfter {
    LEX,
    PARSE,
    SEMANT,
    CODEGEN
};

class CmdlineOptions {
    private:
        std::string sourcefile;
        std::string outfile = "out.S";
        StopAfter stop_after = StopAfter::CODEGEN;

    public:
        CmdlineOptions(int ac, char *av[]);

        std::string get_sourcefile_name() {
            return sourcefile;
        }

        std::string get_outfile_name() {
            return outfile;
        }

        StopAfter get_stop_after() {
            return stop_after;
        }

        void print_usage(int);
};

#endif