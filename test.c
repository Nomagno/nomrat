#include <stdio.h>
#include <time.h>
#include "nomrat.h"

void sleepM(unsigned ms) {
    struct timespec ts;
    ts.tv_sec = ms/1000;
    ts.tv_nsec = (ms%1000)*1000000;
    nanosleep(&ts, NULL);
}

int main(void) {
    ratDeleteAll();

    unsigned flower = ratRegister("flower.glb", "glb");
    unsigned w, h;
    ratGetWH(&w, &h);

    ratClear();
    ratSetXY(0, 0);

    ratPlace(flower, 10, 10, 5, 5);
    ratUpdateRot(flower, 90, 0, 0);

    printf("Terminal size: %u x %u\n", w, h);

    unsigned rotation = 0;
    float x = 0;
    for (unsigned i = 0; i < 100; i++) {
        ratUpdateSimple(flower, x, 0);
        ratUpdateRot(flower, 90, rotation, 0);

        x += 0.5;
        rotation += 15;
        sleepM(33);
    }
    ratDeleteAll();
}
