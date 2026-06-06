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


/****************************************************
NOMRAT ENTITY-COMPONENT-SYSTEM HELPERS
See test.c for detailed usage
****************************************************/

#define IS_3D(_id) (_id < RAT_OBJ_LIMIT)

#ifndef COMPONENT_LIST_TYPE
#error "NomRat Game Error: Define the macro COMPONENT_LIST_TYPE before including"
#endif

#ifndef COMPONENT_LIST_FIELDS
#error "NomRat Game Error: Define the macro COMPONENT_LIST_FIELDS before including"
#endif
// The format must be: component_field_1,component_field_2,component_field_3

COMPONENT_LIST_TYPE g_components;

#define COMMA ,
#define SYSTEM(_name) nomrat_game_system_##_name
#define MAKE_SYSTEM(_name, ...) signed SYSTEM(_name)(__VA_ARGS__)
#define CALL_SYSTEM(_name, ...) SYSTEM(_name)(__VA_ARGS__)

// Sibling code FFFF -> no sibling
#define GET_SIBLING_0(_comp) (_comp.siblings & 0x000000000000FFFF)
#define GET_SIBLING_1(_comp) (_comp.siblings & 0x00000000FFFF0000 >> 16)
#define GET_SIBLING_2(_comp) (_comp.siblings & 0x0000FFFF00000000 >> 32)
#define GET_SIBLING_3(_comp) (_comp.siblings & 0xFFFF000000000000 >> 48)

#define IS_SIBLING(_id) (_id < ENTITY_LIMIT)

#define SET_SIBLING_0(_comp, _id) (_comp.siblings |= _id & 0xFFFF << 0)
#define SET_SIBLING_1(_comp, _id) (_comp.siblings |= _id & 0xFFFF << 16)
#define SET_SIBLING_2(_comp, _id) (_comp.siblings |= _id & 0xFFFF << 32)
#define SET_SIBLING_3(_comp, _id) (_comp.siblings |= _id & 0xFFFF << 48)

// Note: make sure that all component types have these two members: signed id; siblings_t siblings;
// siblings holds up to four IDs of weakly related entities
// The entity limit is 2^16-1
// You can ensure all by copy pasting this before starting the ECS definitions:
// typedef uint64_t siblings_t;
// #define COMPONENT_PRELUDE signed id; siblings_t siblings;
// #define ENTITY_LIMIT 32767
// And start each component struct with "COMPONENT_PRELUDE;"


#define HAS_COMPONENT(_id, _comp_name) (g_components._comp_name[_id].id == (signed)_id)
#define GET_COMPONENT_N(_id, _comp_name) (HAS_COMPONENT(_id, _comp_name) ? &(g_components._comp_name[_id]) : NULL))
#define GET_COMPONENT(_id, _comp_name) (g_components._comp_name[_id])

#include "map.h"
#define ERASE(_comp_name, _id) g_components._comp_name.id = -1;
#define KILL_ENTITY(_id)\
if IS_3D(_id) { ratDelete(_id); }  MAP_UD(ERASE, _id, COMPONENT_LIST_FIELDS)

#define ADD_COMPONENT(_comp_name, _id) g_components._comp_name[_id] = (struct _comp_name){0};\
                                       g_components._comp_name[_id].id = _id;\
                                       g_components._comp_name[_id].siblings = 0xFFFFFFFFFFFFFFFF;
#define ADD_COMPONENTS(_id, ...) MAP_UD(ADD_COMPONENT, _id, __VA_ARGS__)

#define INIT_ARRAY_POS(_comp_name) g_components._comp_name[i].id = -1;\
                                        g_components._comp_name[i].siblings = 0xFFFFFFFFFFFFFFFF;
#define INIT_ECS() for(unsigned i = 0; i < ENTITY_LIMIT; i++) {\
        MAP(INIT_ARRAY_POS, COMPONENT_LIST_FIELDS);\
    }

#endif
