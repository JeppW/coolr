(* COOL program show-casing I/O and string manipulation methods *)

class Main {
    io : IO <- new IO;

    main() : Object {{
        io.out_string("Welcome to the Palindrome Checker\n\n");
        io.out_string("Enter your word: ");

        let s : String <- io.in_string(), result : Bool <- true in {
            let c : Int <- 0, i : Int <- 0, j : Int <- s.length() - 1 in {
                while i < j loop {
                    if s.substr(i, 1) = s.substr(j, 1) then
                        0
                    else 
                        result <- false
                    fi;

                    i <- i + 1;
                    j <- j - 1;
                } pool;
            };

            if result = true then
                io.out_string("The word '".concat(s).concat("' is a palindrome."))
            else 
                io.out_string("The word '".concat(s).concat("' is not a palindrome."))
            fi;
        };
    }};
};