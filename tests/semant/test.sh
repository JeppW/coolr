#!/bin/bash

output_dirname="test-output"

cd grading && mkdir -p ${output_dirname} || exit 1

num_correct_tests=0
num_total_tests=0

for file in *.test; do
    filename=$(basename "$file" .test)

    echo -n "Performing test $filename... ";
    
    # ignore differences in line numbering, implementation details vary
    ../../../coolr "$file" --semant | grep -v '^\s*#' > "${output_dirname}/${filename}_result.txt"
    grep -v '^\s*#' "${filename}.test.out" > "${output_dirname}/${filename}_expected.txt"

    diff "${output_dirname}/${filename}_result.txt" "${output_dirname}/${filename}_expected.txt" > "${output_dirname}/${filename}_diff.txt"

    num_lines=$(wc -l < "${output_dirname}/${filename}_diff.txt")
    if [ "$num_lines" -eq 0 ]; then
        ((num_correct_tests++))
        echo "Passed!"
    else
        echo "Failed."
    fi
    
    ((num_total_tests++))
done

printf "\nPassed %s of %s tests.\n" "$num_correct_tests" "$num_total_tests"
