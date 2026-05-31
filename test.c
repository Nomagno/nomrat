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
#define PATH(_x) "~/path/to/nomrat/assets/objects/" _x

int main(void) {
    ratClearObjects();

    unsigned flower = ratRegister(PATH("flower.glb"), "glb");
    unsigned w, h;
    ratGetWH(&w, &h);

    ratClearText();
    ratSetXY(0, 0);

    ratPlace(flower, 0, 0, 5, 5);
    ratUpdateRot(flower, 90, 0, 0);

    printf("Terminal size: %u x %u\n", w, h);

    unsigned rotation = 0;
    float x = 0;
    float y = 0;
    for (unsigned i = 0; i < 300; i++) {
        ratUpdateSimpleF(flower, x, y);
        ratUpdateRot(flower, 90, rotation, 0);

        x += 0.5;
        y -= 0.2;
        rotation += 15;
        sleepM(33);
    }
    ratClearObjects();
}
