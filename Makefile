CC = g++
CFLAGS = -O2 -std=c++17 -Wall -Wno-parentheses
TARGET = coolr

DIRS = src src/compiler/lexer src/compiler/parser src/compiler/semant src/compiler/codegen src/common src/utils
SRCS = $(wildcard $(addsuffix /*.cpp, $(DIRS)))

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm coolr; rm out.S

.PHONY: clean