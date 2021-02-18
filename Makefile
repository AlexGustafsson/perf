.PHONY: clean

build/main: src/main.c src/perf.c src/perf.h src/utilities.c src/utilities.h
	mkdir -p build
	$(CC) -Wall -fno-omit-frame-pointer -g -o $@ src/main.c src/perf.c src/utilities.c -lcap

clean:
	rm -rf build &>/dev/null || true
