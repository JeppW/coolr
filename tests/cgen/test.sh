#!/bin/bash

output_dirname="test-output"

cd grading && mkdir -p ${output_dirname} || exit 1

num_correct_tests=0
num_total_tests=0

for file in *.cl; do
    filename=$(basename "$file" .cl)

    echo -n "Performing test $filename... ";
    
    ../../../coolr "$file" --cgen && nasm -f elf32 out.S -o program.o && ld -m elf_i386 program.o -o program
    ./program > "${output_dirname}/${filename}_result.txt"

    cp "${filename}.cl.out" "${output_dirname}/${filename}_expected.txt"

    diff "${output_dirname}/${filename}_result.txt" "${filename}.cl.out" > "${output_dirname}/${filename}_diff.txt" 

    num_lines=$(wc -l < "${output_dirname}/${filename}_diff.txt")
    if [ "$num_lines" -eq 0 ]; then
        ((num_correct_tests++))
        echo "Passed!"
    else
        echo "Failed."
    fi
    
    ((num_total_tests++))
done

rm out.S program.o program

printf "\nPassed %s of %s tests.\n" "$num_correct_tests" "$num_total_tests"
