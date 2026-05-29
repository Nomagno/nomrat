#ifndef _NOMRAT_H
#define _NOMRAT_H

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define RAT_OBJ_LIMIT 1024

void rat_internal_pre() { printf("\x1b_ratty;g;"); }
void rat_internal_post() { printf("\x1b\\"); }

char *objects[RAT_OBJ_LIMIT];

void ratGetWH(unsigned *w, unsigned *h) {
    struct winsize wi;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &wi);
    *w = wi.ws_col;
    *h = wi.ws_row;
}
#define NOMRAT_CHECK_ID(_id)\
if (_id >= RAT_OBJ_LIMIT || objects[_id] == NULL)\
    { fprintf(stderr, "NomRat Error: Unknown ID %u\n", _id); exit(1); } \


void ratClear(void) { printf("\x1b[2J"); }

void ratSetXY(unsigned x, unsigned y) { printf("\x1b[%u;%uH", y, x); fflush(stdout); }

unsigned ratRegister(char *path, char *fmt) {
    _Bool obj_limit_reached = 1;
    unsigned retval = 0;
    for (unsigned i = 0; i < RAT_OBJ_LIMIT; i++) {
        if (objects[i] == NULL) {
            objects[i] = path;
            retval = i;
            obj_limit_reached = 0;
            break;
        }
    }
    if (obj_limit_reached) {
        fprintf(stderr, "NomRat Error: Object limit reached (limit is %u)\n", RAT_OBJ_LIMIT);
        exit(1);
    }
    rat_internal_pre();
    printf("r;id=%u;fmt=%s;path=%s", retval, fmt, path);
    rat_internal_post();
    return retval;
}

void ratPlace(unsigned id, unsigned x, unsigned y, unsigned w, unsigned h) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    printf("p;id=%u;row=%u;col=%u;w=%u;h=%u", id, y, x, w, h);
    rat_internal_post();
}

void ratUpdateSimple(unsigned id, unsigned px, unsigned py) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    printf("u;id=%u;px=%u;py=%u", id, px, py);
    rat_internal_post();
}
void ratUpdatePos(unsigned id, unsigned px, unsigned py, unsigned pz) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    printf("u;id=%u;px=%u;py=%u;pz=%u", id, px, py, pz);
    rat_internal_post();
}
void ratUpdateRot(unsigned id, unsigned rx, unsigned ry, unsigned rz) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    /*Rotation in degrees*/
    printf("u;id=%u;rx=%u;ry=%u;rz=%u", id, rx, ry, rz);
    rat_internal_post();
}
void ratUpdateScale(unsigned id, unsigned sx, unsigned sy, unsigned sz) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    printf("u;id=%u;sx=%u;sy=%u;sz=%u", id, sx, sy, sz);
    rat_internal_post();
}
void ratUpdateColorBrightness(unsigned id, uint32_t c, float brightness) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    c = c & 0x00FFFFFF; /*Only lower 24 bits used*/
    printf("u;id=%u;color=%.6x;brightness=%f", id, c, brightness);
    rat_internal_post();
}
void ratUpdateDepth(unsigned id, float depth) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    printf("u;id=%u;depth=%f", id, depth);
    rat_internal_post();
}

void ratDelete(unsigned id) {
    if (id < RAT_OBJ_LIMIT)
        objects[id] = NULL;
    rat_internal_pre();
    printf("d;id=%u", id);
    rat_internal_post();
}
void ratDeleteAll(void) {
    for (unsigned i = 0; i < RAT_OBJ_LIMIT; i++)
        objects[i] = NULL;
    rat_internal_pre();
    printf("d");
    rat_internal_post();
}

#endif
