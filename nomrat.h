#ifndef _NOMRAT_H
#define _NOMRAT_H

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
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

void ratClear(void) { printf("\x1b[2J"); }

void ratSetXY(unsigned x, unsigned y) { printf("\x1b[%u;%uH", y, x); }

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
    if (id >= RAT_OBJ_LIMIT || objects[id] == NULL) {
        fprintf(stderr, "NomRat Error: Unregistered/deregistered ID %u\n", id);
        exit(1);
    }
    rat_internal_pre();
    printf("p;id=%u;row=%u;col=%u;w=%u;h=%u", id, y, x, w, h);
    rat_internal_post();
}

void ratUpdateFull(unsigned id, unsigned px, unsigned py, unsigned pz, unsigned rx, unsigned ry, unsigned rz) {
    if (id >= RAT_OBJ_LIMIT || objects[id] == NULL) {
        fprintf(stderr, "NomRat Error: Unregistered/deregistered ID %u\n", id);
        exit(1);
    }
    rat_internal_pre();
    // Rotation in degrees
    printf("u;id=%u;px=%u;py=%u;pz=%u;rx=%u;ry=%u;rz=%u", id, px, py, pz, rx, ry, rz);
    rat_internal_post();
}

void ratUpdate(unsigned id, unsigned px, unsigned py) {
    if (id >= RAT_OBJ_LIMIT || objects[id] == NULL) {
        fprintf(stderr, "NomRat Error: Unregistered/deregistered ID %u\n", id);
        exit(1);
    }
    rat_internal_pre();
    // Rotation in degrees
    printf("u;id=%u;px=%u;py=%u", id, px, py);
    rat_internal_post();
}

void ratDelete(unsigned id) {
    if (id < RAT_OBJ_LIMIT)
        objects[id] = NULL;
    rat_internal_pre();
    printf("d;id=%u;", id);
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
