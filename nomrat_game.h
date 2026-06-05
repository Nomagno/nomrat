#ifndef _NOMRAT_GAME_H
#define _NOMRAT_GAME_H

// Auxiliary single header library for making games with
// nomrat.h, see test.c for example usage

#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <assert.h>
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

// Terminal size
unsigned g_w, g_h;
// Size in pixels of the terminal
unsigned g_pw, g_ph;
// visually, Y = ratio*X. If ratio is 2.0 (this is the case in most termials),
// this means that the Y axis is visually twice as large as the X axis.
float g_char_ratio;


// Double Vertical precision terminal space (DVpts, DV for short):
// In a typical terminal, characters are twice as tall as they're wide.
// This means that, while 1 character in the y axis might be 1 character,
// it is visually twice the distance as 1 character in the X axis. In DVpts,
// the visual distance is consistent in both axis.
// This is what I choose to call DVpts.
// FROM_DV is a macro that converts Y axis offsets from DVpts to regular terminal space (Rts)
#define TO_REG(_y) (_y/char_ratio)
// TO_DV is the inverse, converts from Rts to DVpts
#define TO_DVP(_y) (_y*char_ratio)
// All animation and placement functions involving the Y axis take Rts:
// ratPlace, ratUpdateSimple, ratUpdateSimpleF, and ratUpdatePos.

float g_epsilon = 0.1;
#define ABS(_x) ((_x < 0) ? -_x : _x)
#define FEQUAL(_x, _y) (ABS(_x - _y) < g_epsilon)

void startGame(void) {
    setRawMode(1);
    setNonblockingInput(1);
    ratClearObjects();
    ratClearText();
    ratSetXY(0, 0);

    ratGetWH(&g_w, &g_h, &g_pw, &g_ph);
    g_char_ratio = (float)(g_ph/g_h)/(float)(g_pw/g_w);

    printf("Ratio: %0.2f\n", g_char_ratio);
    assert(FEQUAL(g_char_ratio, 2.0)
           && "Error: Y axis must be twice the size of X axis to provide consistent visuals");
    ratClearText();
    ratSetXY(0, 0);
}

void endGame(void) {
    setNonblockingInput(0);
    setRawMode(0);
    ratClearObjects();
    ratClearText();
}

#define INPUT_BUFFER_SIZE 64
unsigned char g_read_buffer[INPUT_BUFFER_SIZE];
unsigned readInput(void) {
    return fread(g_read_buffer, 1, INPUT_BUFFER_SIZE, stdin);
}

#endif
