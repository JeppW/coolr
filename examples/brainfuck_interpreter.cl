(* 
    COOL implementation of a Brainfuck interpreter.

    Example BF "Hello World!" code: 
    ++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.
*)

-- COOL doesn't have arrays, so we use a doubly-linked list
-- of Cell objects.
class Cell {
    value : Int <- 0;
    left : Cell;
    right : Cell;

    get_left() : Cell {
        left
    };

    get_right() : Cell {
        right
    };

    set_left(cell : Cell) : Cell {
        left <- cell
    };

    set_right(cell : Cell) : Cell {
        right <- cell
    };

    set_neighbors(l : Cell, r : Cell) : SELF_TYPE {{
        left <- l;
        right <- r;
        self;
    }};

    inc() : Int {
        value <- value + 1
    };

    dec() : Int {
        value <- value - 1
    };

    get_value() : Int {
        value
    };

    set_value(v : Int) : Int {
        value <- v
    };
};

-- Implementing support of Brainfuck loops requires us to parse the
-- program and identify matching square brackets. Since COOL does not 
-- support data structures like maps and stacks, we have to implement
-- these.

class BracketStackItem {
    next : BracketStackItem;
    value : Int;

    get_next() : BracketStackItem {
        next
    };

    set_next(n : BracketStackItem) : BracketStackItem {
        next <- n
    };

    get_value() : Int {
        value
    };

    set_value(v : Int) : Int {
        value <- v
    };
};

class BracketStack {
    top : BracketStackItem;

    push(s : BracketStackItem) : Object {
        let tmp_item : BracketStackItem <- top in {
            top <- s;
            s.set_next(top);
        }
    };

    pop() : BracketStackItem {
        let tmp_item : BracketStackItem <- top in {
            top <- top.get_next();
            tmp_item;
        }
    };

    get() : BracketStackItem {
        top
    };
};

class BracketMapItem {
    start : Int;
    end : Int;
    next : BracketMapItem;

    get_start() : Int {
        start
    };

    set_start(s : Int) : Int {
        start <- s
    };

    get_end() : Int {
        end
    };

    set_end(e : Int) : Int {
        end <- e
    };

    get_next() : BracketMapItem {
        next
    };

    set_next(n : BracketMapItem) : BracketMapItem {
        next <- n
    };
};

class BracketMap inherits IO {
    head : BracketMapItem;
    stack : BracketStack <- new BracketStack;

    add(start : Int, end : Int) : Object {{
        -- add to map by finding the last item in the linked list
        let current : BracketMapItem <- head in {
            if isvoid current then {
                -- handle the case where head is void
                let new_item : BracketMapItem <- new BracketMapItem in {
                    new_item.set_start(start);
                    new_item.set_end(end);
                    head <- new_item;
                };
            } else {
                while not (isvoid current.get_next()) loop 
                    current <- current.get_next()
                pool;

                let new_item : BracketMapItem <- new BracketMapItem in {
                    new_item.set_start(start);
                    new_item.set_end(end);
                    current.set_next(new_item);
                };
            } fi;
        };
    }};

    get_end(start : Int) : Int {
        let current : BracketMapItem <- head in {
            while not current.get_start() = start loop 
                current <- current.get_next()
            pool;

            current.get_end();
        }
    };

    get_start(end : Int) : Int {
        let current : BracketMapItem <- head in {
            while not current.get_end() = end loop 
                current <- current.get_next()
            pool;

            current.get_start();
        }
    };

    init(program : String) : SELF_TYPE {{
        -- build the bracket map from the program string
        let c : Int <- 0, op : String in {
            while c < program.length() loop {
                op <- program.substr(c, 1);

                if op = "[" then {
                    stack.push(new BracketStackItem);
                    stack.get().set_value(c);
                } else 
                    0
                fi;

                if op = "]" then
                    add(stack.pop().get_value(), c)
                else 
                    0
                fi;

                c <- c + 1;
            } pool;
        };

        self;
    }};
};

class Main inherits IO {
    io : IO <- new IO;
    cell : Cell;
    cellptr : Int;
    num_cells : Int <- 100;

    int_to_char(value : Int) : String {
        -- This method is sinful, but it is the only way I
        -- know of converting ints to chars in COOL
        if value =  9 then "\t" else
        if value = 10 then "\n" else
        if value = 13 then "\r" else
        if value = 32 then " " else
        if value = 33 then "!" else
        if value = 34 then "\"" else
        if value = 35 then "#" else
        if value = 36 then "$" else
        if value = 37 then "%" else
        if value = 38 then "&" else
        if value = 39 then "'" else
        if value = 40 then "(" else
        if value = 41 then ")" else
        if value = 42 then "*" else
        if value = 43 then "+" else
        if value = 44 then "," else
        if value = 45 then "-" else
        if value = 46 then "." else
        if value = 47 then "/" else
        if value = 48 then "0" else
        if value = 49 then "1" else
        if value = 50 then "2" else
        if value = 51 then "3" else
        if value = 52 then "4" else
        if value = 53 then "5" else
        if value = 54 then "6" else
        if value = 55 then "7" else
        if value = 56 then "8" else
        if value = 57 then "9" else
        if value = 58 then ":" else
        if value = 59 then ";" else
        if value = 60 then "<" else
        if value = 61 then "=" else
        if value = 62 then ">" else
        if value = 63 then "?" else
        if value = 64 then "@" else
        if value = 65 then "A" else
        if value = 66 then "B" else
        if value = 67 then "C" else
        if value = 68 then "D" else
        if value = 69 then "E" else
        if value = 70 then "F" else
        if value = 71 then "G" else
        if value = 72 then "H" else
        if value = 73 then "I" else
        if value = 74 then "J" else
        if value = 75 then "K" else
        if value = 76 then "L" else
        if value = 77 then "M" else
        if value = 78 then "N" else
        if value = 79 then "O" else
        if value = 80 then "P" else
        if value = 81 then "Q" else
        if value = 82 then "R" else
        if value = 83 then "S" else
        if value = 84 then "T" else
        if value = 85 then "U" else
        if value = 86 then "V" else
        if value = 87 then "W" else
        if value = 88 then "X" else
        if value = 89 then "Y" else
        if value = 90 then "Z" else 
        if value = 97 then "a" else
        if value = 98 then "b" else
        if value = 99 then "c" else
        if value = 100 then "d" else
        if value = 101 then "e" else
        if value = 102 then "f" else
        if value = 103 then "g" else
        if value = 104 then "h" else
        if value = 105 then "i" else
        if value = 106 then "j" else
        if value = 107 then "k" else
        if value = 108 then "l" else
        if value = 109 then "m" else
        if value = 110 then "n" else
        if value = 111 then "o" else
        if value = 112 then "p" else
        if value = 113 then "q" else
        if value = 114 then "r" else
        if value = 115 then "s" else
        if value = 116 then "t" else
        if value = 117 then "u" else
        if value = 118 then "v" else
        if value = 119 then "w" else
        if value = 120 then "x" else
        if value = 121 then "y" else
        if value = 122 then "z" else 
        if value = 123 then "{" else
        if value = 124 then "|" else
        if value = 125 then "}" else
        if value = 126 then "~" else "?"
        fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi
        fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi
        fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi
        fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi
        fi fi fi fi fi fi fi fi
    };

    init() : Object {{
        -- init cells
        let tmp_cell : Cell in {
            cell <- new Cell;
            tmp_cell <- cell;

            cell.set_right(new Cell);

            let c : Int <- 0 in {
                while c < num_cells - 2 loop {
                    cell <- cell.get_right().set_neighbors(cell, new Cell);
                    c <- c + 1;
                } pool;
            };

            cell.get_right().set_left(cell);
            cell <- tmp_cell;
        };

        cellptr <- 0;
    }};

    main() : Object {{
        init();

        io.out_string("Reading Brainfuck program from stdin...\n\n");

        let bf_program : String <- io.in_string(), c : Int <- 0, op : String, map : BracketMap <- new BracketMap in {
            map.init(bf_program);
            
            while c < bf_program.length() loop {
                op <- bf_program.substr(c, 1);

                if op = ">" then {
                    if cellptr = num_cells - 1 then {
                        io.out_string("Tried to access out-of-bounds cell (right side)\n");
                        abort();
                    } else {
                        cellptr <- cellptr + 1;
                        cell <- cell.get_right();
                    } fi;
                } else 
                    0
                fi;

                if op = "<" then {
                    if cellptr = 0 then {
                        io.out_string("Tried to access out-of-bounds cell (left side)\n");
                        abort();
                    } else {
                        cellptr <- cellptr - 1;
                        cell <- cell.get_left();
                    } fi;
                } else 
                    0
                fi;

                if op = "+" then
                    cell.inc()
                else 
                    0
                fi;

                if op = "-" then
                    cell.dec()
                else 
                    0
                fi;

                if op = "." then
                    io.out_string(int_to_char(cell.get_value()))
                else 
                    0
                fi;

                if op = "," then
                    cell.set_value(io.in_int())
                else 
                    0
                fi;

                if op = "[" then
                    if cell.get_value() = 0 then
                        c <- map.get_end(c)
                    else 
                        0
                    fi
                else 
                    0
                fi;

                if op = "]" then
                    if not cell.get_value() = 0 then
                        c <- map.get_start(c)
                    else 
                        0
                    fi
                else 
                    0
                fi;

                c <- c + 1;
            } pool;
        };
    }};
};