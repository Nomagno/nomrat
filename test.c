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

void startGame(void) {
    setNonblockingInput(1);
    setRawMode(1);
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

// !!! MODIFY THIS WITH YOUR PATH TO nomrat ASSETS
#define PATH(_x) "~/path/to/nomrat/assets/objects/" _x

unsigned real_width, real_height;
unsigned game_width=24, game_height=80;

int main(void) {
    startGame();
    ratGetWH(&real_width, &real_height);

    unsigned flower = ratRegister(PATH("flower.glb"), "glb");
    ratPlace(flower, 3, 3, 4, 4);
    ratUpdateRot(flower, 90, 0, 0);

    printf("Terminal size: %u x %u. W,A,S,D: 2D movement; Q,E: 3D movement; B: quit\n", real_width, real_height);

    unsigned rotation = 0;
    float x = 0;
    float y = 0;
    float z = 0;

    _Bool game_active = 1;
    while(game_active) {
        ratUpdateSimpleF(flower, x, y);
        ratUpdateZ(flower, z);
        ratUpdateRot(flower, 90, rotation, 0);
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
