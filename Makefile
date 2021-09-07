common_deps = obj/cpu.o obj/memory.o obj/screen.o

flags = -Wall -Iinclude -g
sdl = `sdl2-config --cflags --libs`

.PHONY: all clean build test

all: clean dirs build

test: build/test
	build/test

build: build/emulator
	build/emulator roms/invaders

clean:
	rm -rf obj/ build/

dirs:
	mkdir obj/ build/

build/emulator: obj/main.o $(common_deps)
	gcc $(flags) -o $@ obj/main.o $(common_deps) $(sdl)

build/test: obj/test.o obj/cpu.o obj/memory.o
	gcc $(flags) -o $@ obj/test.o $(common_deps) $(sdl)

obj/main.o: src/main.c include/cpu.h include/memory.h include/screen.h
	gcc $(flags) -c src/main.c -o $@ $(sdl)

obj/cpu.o: src/cpu.c include/cpu.h include/memory.h include/screen.h
	gcc $(flags) -c src/cpu.c -o $@ 

obj/memory.o: src/memory.c include/memory.h
	gcc $(flags) -c src/memory.c -o $@ $(sdl)

obj/screen.o: src/screen.c include/screen.h
	gcc $(flags) -c src/screen.c -o $@ $(sdl)

obj/test.o: src/test.c include/cpu.h include/memory.h
	gcc $(flags) -c src/test.c -o $@



