#include <SDL.h>

#define USAGE "Usage: ./build/emulator [rom].\n"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, USAGE);
        return 1;
    }
}
