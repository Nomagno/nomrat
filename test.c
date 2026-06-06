#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "nomrat.h"

typedef uint64_t siblings_t;
#define COMPONENT_PRELUDE signed id; siblings_t siblings;
#define ENTITY_LIMIT 32767

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

#include "nomrat_game.h"

// !!! MODIFY THIS WITH YOUR PATH TO nomrat ASSETS
#define PATH(_x) "~/path/to/nomrat/assets/objects/" _x

uint32_t getInput(void) {
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
    for (unsigned id = 0; id < ENTITY_LIMIT; id++) {
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
    for (unsigned id = 0; id < ENTITY_LIMIT; id++) {
        if (!IS_3D(id))
            continue;
        if (!HAS_COMPONENT(id, position))
            continue;
        GET_COMPONENT(id, position).ry += 15;
        GET_COMPONENT(id, position).ry = fmod(GET_COMPONENT(id, position).ry, 360);

    }
    return 0;
}

MAKE_SYSTEM(render_3d) {
    rat_defer_commands = 1;
    for (unsigned id = 0; id < RAT_OBJ_LIMIT; id++) {
        if (!IS_3D(id))
            continue;
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
    rat_defer_commands = 1;
    for (unsigned i = 0; i < 64; i++) {
        unsigned id = ratRegister("cuteFlower", PATH("flower.glb"), "glb");
        ADD_COMPONENTS(id, position, origin);
        unsigned x = 3 + (i%16)*3, y = 5 + (i/16)*3;
        unsigned w = 4, h = 4;
        ratPlace(id, x, y, w, h);
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
