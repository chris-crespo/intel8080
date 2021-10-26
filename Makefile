common_deps = obj/cpu.o

flags = -Wall -Wextra -Iinclude -g
sdl = `sdl2-config --cflags --libs`

.PHONY: invaders test clean dirs

invaders: dirs build/invaders
	build/invaders

test: dirs build/test
	build/test

clean:
	rm -rf obj/ build/

dirs:
	@mkdir -p obj/ build/

build/invaders: obj/invaders.o obj/cpu.o
	gcc $(flags) -o $@ obj/invaders.o obj/cpu.o

obj/invaders.o: invaders/main.c
	gcc $(flags) -c invaders/main.c -o $@

build/test: obj/test.o obj/cpu.o 
	gcc $(flags) -o $@ obj/test.o obj/cpu.o

obj/cpu.o: core/cpu.c include/cpu.h
	gcc $(flags) -c core/cpu.c -o $@ 

obj/test.o: core/test.c include/cpu.h 
	gcc $(flags) -c core/test.c -o $@



