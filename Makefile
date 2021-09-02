flags = -Wall -Iinclude -g
sdl = `sdl2-config --cflags --libs`

.PHONY: all clean build

all: clean dirs build

build: dirs build/emulator
	build/emulator

clean:
	rm -rf obj/ build/

dirs:
	mkdir obj/ build/

build/emulator: obj/main.o
	gcc $(flags) -o $@ obj/*.o

obj/main.o: src/main.c
	gcc $(flags) -c src/main.c -o $@ $(sdl)




