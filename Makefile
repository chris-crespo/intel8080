common_deps = obj/cpu.o

flags = -Wall -Wextra -Iinclude -g
sdl = `sdl2-config --cflags --libs`

.PHONY: all clean build test

test: dirs build/test
	build/test

clean:
	rm -rf obj/ build/

dirs:
	@mkdir -p obj/ build/

build/test: obj/test.o obj/cpu.o 
	gcc $(flags) -o $@ obj/test.o obj/cpu.o

obj/cpu.o: core/cpu.c include/cpu.h
	gcc $(flags) -c core/cpu.c -o $@ 

obj/test.o: core/test.c include/cpu.h 
	gcc $(flags) -c core/test.c -o $@



