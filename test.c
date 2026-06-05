#include <stdio.h>
#include "nomrat.h"
#include "nomrat_game.h"

// !!! MODIFY THIS WITH YOUR PATH TO nomrat ASSETS
#define PATH(_x) "~/Documents/Git/nomrat/assets/objects/" _x

int main(void) {
    startGame();

    printf("Terminal size in cells; pixels: %ux%u; %ux%u. Ratio: %0.4f\n"
           "W,A,S,D: 2D movement; Q,E: 3D movement; B: quit\n",
           g_w, g_h, g_pw, g_ph, g_char_ratio);

    for (unsigned i = 0; i < g_h-5; i++) {
        for (unsigned  j = 0; j < g_w; j++) {
            printf(" ");
        }
        printf("\n");
     }


    unsigned flowers[RAT_OBJ_LIMIT];
    rat_defer_commands = 1;
    for (unsigned i = 0; i < RAT_OBJ_LIMIT; i++) {
        flowers[i] = ratRegister(PATH("flower.glb"), "glb");
        ratPlace(flowers[i], 3 + (i%16)*3, 5 + (i/16)*3, 4, 4);
        ratUpdateRot(flowers[i], 90, 0, 0);
    }
    ratForce();


    unsigned rotation = 0;
    float x = 0;
    float y = 0;
    float z = 0;

    _Bool game_active = 1;
    while(game_active) {
        rat_defer_commands = 1;
        for (unsigned i = 0; i < RAT_OBJ_LIMIT; i++) {
            ratUpdateSimpleF(flowers[i], x, y);
            ratUpdateZ(flowers[i], z);
            ratUpdateRot(flowers[i], 90, rotation, 0);
        }
        ratForce();

        // The read buffer is large in order to avoid blocking the program in input handling easily,
        // but we only take the first input during the frame and the rest will be ignored
        // for this example. The return value of readInput() is the buffer length.
        if (readInput()) switch(g_read_buffer[0]) {
        case 'a':
            x -= 1;
            break;
        case 'd':
            x += 1;
            break;
        case 'w':
            y += 1;
            break;
        case 's':
            y -= 1;
            break;
        case 'q':
            z -= 5;
            break;
        case 'e':
            z += 5;
            break;
        case 'b':
            game_active = 0;
            continue;
            break;
        default:
            break;
        }

        rotation += 15;

        // ~30FPS, frame time = 33ms
        sleepM(33);
    }

    endGame();
}
