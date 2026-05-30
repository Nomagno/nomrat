#ifndef _NOMRAT_H
#define _NOMRAT_H

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define RAT_OBJ_LIMIT 1024
#define NOMRAT_CHECK_ID(_id)\
if (_id >= RAT_OBJ_LIMIT || rat_internal_objects[_id] == NULL)\
    { fprintf(stderr, "NomRat Error: Unknown ID %u\n", _id); exit(1); } \

void rat_internal_pre() { printf("\x1b_ratty;g;"); }
void rat_internal_post() { printf("\x1b\\"); fflush(stdout); }
char *rat_internal_objects[RAT_OBJ_LIMIT];
unsigned rat_internal_w=1, rat_internal_h=1;

// Stores the terminal width in columns into w,
// and the height in rows into h.
// It is recommended to call this at program startup
// for the internal nomrat terminal dimenstions to be
// filled in
void ratGetWH(unsigned *w, unsigned *h) {
    struct winsize wi;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &wi);
    *w = wi.ws_col;
    *h = wi.ws_row;
    rat_internal_w = wi.ws_col;
    rat_internal_h = wi.ws_row;
}

// Deletes ALL objects
void ratClearObjects(void) {
    for (unsigned i = 0; i < RAT_OBJ_LIMIT; i++)
        rat_internal_objects[i] = NULL;
    rat_internal_pre();
    printf("d");
    rat_internal_post();
}

// Clears screen (text)
void ratClearText(void) { printf("\x1b[2J"); fflush(stdout); }

// Sets cursor position
void ratSetXY(unsigned x, unsigned y) { printf("\x1b[%u;%uH", y, x); fflush(stdout); }

// Registers object with path (path relative to ratty's assets/objects/ folder, absolute paths not supported)
// and where format is one of "obj", "glb".
// Returns the integer handle (ID) of the object for use with the rest of commands
unsigned ratRegister(char *path, char *fmt) {
    _Bool obj_limit_reached = 1;
    unsigned retval = 0;
    for (unsigned i = 0; i < RAT_OBJ_LIMIT; i++) {
        if (rat_internal_objects[i] == NULL) {
            rat_internal_objects[i] = path;
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

// Places object at position x,y with size w,h
void ratPlace(unsigned id, unsigned x, unsigned y, unsigned w, unsigned h) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    printf("p;id=%u;row=%u;col=%u;w=%u;h=%u", id, y, x, w, h);
    rat_internal_post();
}

// Note: According to Ratty:
//       1 unit in the x axis = 1 screen width
//       1 unit in the y axis = 1 screen height
//       1 unit in the z axis = Ratty hardcodes them internally
//              (though, for reference by default the screen is at depth 18,
//              so probably keep any total z displacement to <18)
// But this interface takes care that 1 unit = 1 character for the xy plane, the z axis is left untouched
// Updates position OFFSET of object. That is, the offset from its original position
void ratUpdatePos(unsigned id, float px, float py, float pz) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    printf("u;id=%u;px=%f;py=%f;pz=%f", id, px/rat_internal_w, py/rat_internal_h, pz);
    rat_internal_post();
}

// Updates position of object, integer xy plane version for terminal-style jagged movement
void ratUpdateSimple(unsigned id, int px, int py) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    printf("u;id=%u;px=%f;py=%f", id, ((float)px)/rat_internal_w, ((float)py)/rat_internal_h);
    rat_internal_post();
}

// Updates position of object, float xy plane version for smooth movement
void ratUpdateSimpleF(unsigned id, float px, float py) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    printf("u;id=%u;px=%f;py=%f", id, px/(float)rat_internal_w, py/(float)rat_internal_h);
    rat_internal_post();
}

// Updates rotation of object, where the rotation is in degrees
void ratUpdateRot(unsigned id, float rx, float ry, float rz) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    printf("u;id=%u;rx=%f;ry=%f;rz=%f", id, rx, ry, rz);
    rat_internal_post();
}

// Updates scale of object
void ratUpdateScale(unsigned id, float sx, float sy, float sz) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    printf("u;id=%u;sx=%f;sy=%f;sz=%f", id, sx, sy, sz);
    rat_internal_post();
}

// Updates the color (in format 0xRRGGBB) and the brightnes [0.0,1.0] of the object
void ratUpdateColorBrightness(unsigned id, uint32_t c, float brightness) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    c = c & 0x00FFFFFF; /*Only lower 24 bits used*/
    printf("u;id=%u;color=%.6x;brightness=%f", id, c, brightness);
    rat_internal_post();
}

// Updates the depth offset
void ratUpdateDepth(unsigned id, float depth) {
    NOMRAT_CHECK_ID(id);
    rat_internal_pre();
    printf("u;id=%u;depth=%f", id, depth);
    rat_internal_post();
}

// Deletes object
void ratDelete(unsigned id) {
    if (id < RAT_OBJ_LIMIT)
        rat_internal_objects[id] = NULL;
    rat_internal_pre();
    printf("d;id=%u", id);
    rat_internal_post();
}
#endif
