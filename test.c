#include <stdio.h>
#include "nomrat.h"

int main(void) {
    ratDeleteAll();

    unsigned flower = ratRegister("flower.glb", "glb");
    unsigned w, h;
    ratGetWH(&w, &h);

    ratClear();
    ratSetXY(0, 0);
    ratPlace(flower, 5, 5, 50, 50);
    printf("Terminal size: %u x %u\n", w, h);
}
