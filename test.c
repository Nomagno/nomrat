#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "nomrat.h"

// Helper mode
#include "nomrat_game.h"
struct position {
    COMPONENT_PRELUDE;
    float x, y, z;
    float rx, ry, rz;
};

struct origin {
    COMPONENT_PRELUDE;
    unsigned x, y, z;
    float ox, oy, oz;
    float rx, ry, rz;
};

struct Components {
    struct position position[ENTITY_LIMIT];
    struct origin origin[ENTITY_LIMIT];
};

#define COMPONENT_LIST_TYPE struct Components
#define COMPONENT_LIST_FIELDS position,origin

// load the library after providing the two user-defined macros
#define NOMRAT_GAME_LOAD
#include "nomrat_game.h"

// !!! MODIFY THIS WITH YOUR PATH TO nomrat ASSETS
#define PATH(_x) "~/Documents/Git/nomrat/assets/objects/" _x

unsigned getInput(void) {
    // The read buffer is large in order to avoid blocking the program in input handling easily,
    // but we only take the first input during the frame and the rest will be ignored
    // for this example. The return value of readInput() is the buffer length.
    if (readInput())
        return g_read_buffer[0];
    else
        return 0;
}

void makeSpace(void) {
    for (unsigned i = 0; i < g_h; i++) {
        for (unsigned  j = 0; j < g_w; j++) {
            printf(" ");
        }
        printf("\n");
    }
}

_Bool global_game_active = 1;

MAKE_SYSTEM(handle_input) {
    char input = getInput();
    ITERATE_OVER_COMPONENT_LIST(id) {
        if (!HAS_COMPONENT(id, position))
            continue;

        struct position *p = &GET_COMPONENT(id, position);
        switch(input) {
        case 'a':
            p->x -= 1;
            break;
        case 'd':
            p->x += 1;
            break;
        case 'w':
            p->y += 1;
            break;
        case 's':
            p->y -= 1;
            break;
        case 'q':
            p->z -= 5;
            break;
        case 'e':
            p->z += 5;
            break;
        case 'b':
            global_game_active = 0;
            continue;
            break;
        default:
            break;
        }
    }
    return 0;
}

MAKE_SYSTEM(animate_spin) {
    ITERATE_OVER_COMPONENT_LIST(id) {
        // optimization: all the ratty object IDs come first, this is
        // an engine guarantee, so if we find a non-3D object we can exit the loop
        if (!IS_3D(id))
            break;
        if (!HAS_COMPONENT(id, position))
            continue;
        float *ry = &GET_COMPONENT(id, position).ry;
        *ry += 15;
        *ry = fmod(*ry, 360);
    }
    return 0;
}

MAKE_SYSTEM(render_3d) {
    rat_defer_commands = 1;
    ITERATE_OVER_COMPONENT_LIST(id) {
        // optimization: all the ratty object IDs come first, this is
        // an engine guarantee, so if we find a non-3D object we can exit the loop
        if (!IS_3D(id))
            break;
        if (!HAS_COMPONENT(id, origin))
            continue;
        if (!HAS_COMPONENT(id, position))
            continue;
        struct origin *o = &GET_COMPONENT(id, origin);
        struct position *p = &GET_COMPONENT(id, position);
        ratUpdatePos(id, o->ox+p->x, o->oy+p->y, o->oz+p->z);
        ratUpdateRot(id, o->rx+p->rx, o->ry+p->ry, o->rz+p->rz);
        //printf("Rendered for %d: %f,%f,%f; %f,%f,%f\n", id, o->x+o->ox+p->x, o->y+o->oy+p->y, o->z+o->oz+p->z, o->rx+p->rx, o->ry+p->ry, o->rz+p->rz);
    }
    ratForce();
    return 0;
}

int main() {
    startGame();
    makeSpace();
    INIT_ECS();
    ratSetXY(0, 0);
    printf("Terminal size in cells: %u x %u; in pixels: %u x %u\n"
           "Controls: WASD for 2D movement, QE for 3D movement, B to quit.\n",
            g_w, g_h, g_pw, g_ph);
    rat_defer_commands = 1;
    for (unsigned i = 0; i < 64; i++) {
        //ratRegister and ratPlace must be called explicitly, but not ratDelete.
        //ratDelete is handled by the KILL_ENTITY(entity_id) macro.
        unsigned id = ratRegister("cuteFlower", PATH("flower.glb"), "glb");
        unsigned x = 3 + (i%16)*3, y = 5 + (i/16)*3;
        unsigned w = 4, h = 4;
        ratPlace(id, x, y, w, h);

        ADD_COMPONENTS(id, position, origin);
        GET_COMPONENT(id, origin).x = x;
        GET_COMPONENT(id, origin).y = y;
        // To rotate the flower.glb model correctly for visualizing
        GET_COMPONENT(id, origin).rx = 90;
    }
    ratForce();
    while(global_game_active) {
        CALL_SYSTEM(handle_input);
        CALL_SYSTEM(animate_spin);
        CALL_SYSTEM(render_3d);
        // ~30FPS, frame time = 33ms
        sleepM(33);
    }
    endGame();
}
