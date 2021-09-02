flags = -Wall -Iinclude -g
sdl = `sdl2-config --cflags --libs`

.PHONY: all clean build

all: clean dirs build

build: dirs build/emulator
	build/emulator space_invaders

clean:
	rm -rf obj/ build/

dirs:
	mkdir obj/ build/

build/emulator: obj/main.o obj/screen.o
	gcc $(flags) -o $@ obj/*.o $(sdl)

obj/main.o: src/main.c include/screen.h
	gcc $(flags) -c src/main.c -o $@ $(sdl)

obj/screen.o: src/screen.c include/screen.h
	gcc $(flags) -c src/screen.c -o $@ $(sdl)




