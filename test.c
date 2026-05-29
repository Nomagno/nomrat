#include <stdio.h>
#include "nomrat.h"

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
}
