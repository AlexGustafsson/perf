.PHONY: clean

build/main: src/main.c src/perf.c src/perf.h
	mkdir -p build
	$(CC) -fno-omit-frame-pointer -g -o $@ src/main.c src/perf.c

clean:
	rm -rf build &>/dev/null || true
