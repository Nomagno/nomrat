#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include "nomrat.h"

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

// !!! MODIFY THIS WITH YOUR PATH TO nomrat ASSETS
#define PATH(_x) "~/Documents/Git/nomrat/assets/objects/" _x

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

int main(void) {
    startGame();
    ratGetWH(&w, &h, &pw, &ph);

    printf("Terminal size in cells: %u x %u.\n"
           "Terminal size in pixels: %u x %u\n"
           "W,A,S,D: 2D movement; Q,E: 3D movement; B: quit\n",
           w, h, pw, ph);

    for (unsigned i = 0; i < h-5; i++) {
        for (unsigned  j = 0; j < w; j++) {
            printf(" ");
        }
        printf("\n");
     }


    #define FLOWER_C 64
    unsigned flowers[FLOWER_C];
    defer_commands = 1;
    for (unsigned i = 0; i < FLOWER_C; i++) {
        flowers[i] = ratRegister(PATH("flower.glb"), "glb");
        ratPlace(flowers[i], 3 + (i%16)*3, 5 + (i/16)*3, 4, 4);
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
        for (unsigned i = 0; i < FLOWER_C; i++) {
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
