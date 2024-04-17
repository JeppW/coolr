#!/bin/sh

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <infile> <outfile>"
    exit 1
fi

infile=$1
outfile=$2

nasm -f elf32 $infile -o program.o && ld -m elf_i386 program.o -o $outfile