# Disable echoing of commands
MAKEFLAGS += --silent

export CCFLAGS := $(CCFLAGS) -O3 -Wall -Wextra -pedantic -Wno-unused-parameter -fno-omit-frame-pointer -g

source := $(shell find * -type f -name "*.c" -not -path "build/*")
headers := $(shell find * -type f -name "*.h" -not -path "build/*")

.PHONY: build library format clean

build: build/main

library: build/lib/perf/libperf.a lib/perf.h lib/utilities.h
	mkdir -p build/include/perf/
	cp lib/perf.h lib/utilities.h build/include/perf

build/lib/perf/libperf.a: build/perf.o build/utilities.o
	mkdir -p $(dir $@)
	$(AR) rcs $@ $^

build/perf.o: lib/perf.c lib/perf.h
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -c -o $@ $<

build/utilities.o: lib/utilities.c lib/utilities.h
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -c -o $@ $<

build/main: library src/main.c
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -o $@ src/main.c -I build/include -L build/lib/perf -lperf -lcap

# Create the compilation database for llvm tools
compile_commands.json: Makefile
	# compiledb is instqalled using: pip install compiledb
	compiledb -n make

# Format code according to .clang-format
format: compile_commands.json
	clang-format -style=file -i $(source) $(headers)

clean:
	rm -rf build &>/dev/null || true
