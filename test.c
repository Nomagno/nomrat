#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include "nomrat.h"

// !!! MODIFY THIS WITH YOUR PATH TO nomrat ASSETS
#define PATH(_x) "~/path/to/nomrat/assets/objects/" _x

void sleepM(unsigned ms) {
    struct timespec ts;
    ts.tv_sec = ms/1000;
    ts.tv_nsec = (ms%1000)*1000000;
    nanosleep(&ts, NULL);
}

void setRawMode(_Bool enable) {
    struct termios old, new;

    tcgetattr(STDIN_FILENO, &old);
    new = old;

    if (enable) {
        new.c_lflag &= ~(ICANON | ECHO);
    } else {
        new.c_lflag |= ICANON | ECHO;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &new);
}

void setNonblockingInput(_Bool enable) {
    if (enable) {
        fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
    } else {
        fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & (~O_NONBLOCK));
    }
}

// Terminal size
unsigned w, h;
// Size in pixels of the terminal
unsigned pw, ph;

void startGame(void) {
    setRawMode(1);
    setNonblockingInput(1);
    ratClearObjects();
    ratClearText();
    ratSetXY(0, 0);
}

void endGame(void) {
    setNonblockingInput(0);
    setRawMode(0);
    ratClearObjects();
    ratClearText();
}

unsigned char read_buffer[64];
unsigned readInput(void) {
    return fread(read_buffer, 1, 64, stdin);
}

void makeSpace(void) {
    for (unsigned i = 0; i < h-5; i++) {
        for (unsigned  j = 0; j < w; j++) {
            printf(" ");
        }
        printf("\n");
     }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./test object_count\n");
        return 1;
    }
    unsigned object_count = strtol(argv[1], NULL, 10);
    if (object_count > 4096) {
        fprintf(stderr, "Error: object_count must be between 0 and 4096\n");
        return 1;
    }


    startGame();
    ratGetWH(&w, &h, &pw, &ph);

    printf("Terminal size in cells: %u x %u; in pixels: %u x %u\n"
           "Object count: %u\n"
           "W,A,S,D: 2D movement; Q,E: 3D movement; B: quit\n",
           w, h, pw, ph, object_count);

    unsigned flowers[object_count];
    defer_commands = 1;
    for (unsigned i = 0; i < object_count; i++) {
        flowers[i] = ratRegister(PATH("flower.glb"), "glb");
        ratPlace(flowers[i], 3 + (i%100), 5 + (i/100), 4, 4);
        ratUpdateRot(flowers[i], 90, 0, 0);
    }
    ratForce();


    unsigned rotation = 0;
    float x = 0;
    float y = 0;
    float z = 0;

    _Bool game_active = 1;
    while(game_active) {
        defer_commands = 1;
        for (unsigned i = 0; i < object_count; i++) {
            ratUpdateSimpleF(flowers[i], x, y);
            ratUpdateZ(flowers[i], z);
            ratUpdateRot(flowers[i], 90, rotation, 0);
        }
        ratForce();
        if (readInput()) switch(read_buffer[0]){
        case 'a':
            x -= 1;
            break;
        case 'd':
            x += 1;
            break;
        case 'w':
            y += 1;
            break;
        case 's':
            y -= 1;
            break;
        case 'q':
            z -= 5;
            break;
        case 'e':
            z += 5;
            break;
        case 'b':
            game_active = 0;
            continue;
            break;
        default:
            break;
        }

        rotation += 15;
        sleepM(33);
    }

    endGame();
}
