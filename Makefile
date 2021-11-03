invaders_deps = obj/invaders.o obj/cpu.o obj/screen.o obj/input.o obj/shift.o

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

build/invaders: $(invaders_deps)
	gcc $(flags) -o $@ $(invaders_deps) $(sdl)

obj/invaders.o: invaders/main.c
	gcc $(flags) -c invaders/main.c -o $@ $(sdl)

obj/screen.o: invaders/screen.c include/screen.h
	gcc $(flags) -c invaders/screen.c -o $@ $(sdl)

obj/input.o: invaders/input.c include/input.h
	gcc $(flags) -c invaders/input.c -o $@ $(sdl)

obj/shift.o: invaders/shift.c include/shift.h
	gcc $(flags) -c invaders/shift.c -o $@

build/test: obj/test.o obj/cpu.o 
	gcc $(flags) -o $@ obj/test.o obj/cpu.o

obj/cpu.o: core/cpu.c include/cpu.h
	gcc $(flags) -c core/cpu.c -o $@ 

obj/disassembler.o: core/disassembler.c include/disassembler.h
	gcc $(flags) -c core/disassembler.c -o $@

obj/test.o: core/test.c include/cpu.h 
	gcc $(flags) -c core/test.c -o $@



