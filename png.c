#include "raylib.h"
#include <stdio.h>
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024
#define FPS 60

int main() {

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voxel space rendering");
    
    Image map  = LoadImage("resources/map0.height.gif");
    Color *mapColors = LoadImageColors(map);
    for (size_t i = 0; i < 1024*1024; i++) {
        printf ("%u %u %u %u\n", mapColors[i].r, mapColors[i].g, mapColors[i].b, mapColors[i].a);
    }

    SetTargetFPS(FPS);

    while(!WindowShouldClose()) {
        ClearBackground(RAYWHITE);

        BeginDrawing();

            ClearBackground(WHITE);
            for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
                for (size_t x = 0; x < SCREEN_WIDTH; x++) {
                    DrawPixel(x, y, mapColors[SCREEN_WIDTH * y + x]);
                }
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;


}
