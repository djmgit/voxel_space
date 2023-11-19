#include "raylib.h"
#include <stdio.h>
#include <math.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define FPS 60
#define MAP_N 1024
#define SCALE_FACTOR 100.0

Color *colorMap = NULL;
Color *heightMap = NULL;

typedef struct {
    float x;
    float y;
    float height;
    float angle;
    float zfar;
} camera_t;

camera_t camera = {
    .x = 512,
    .y = 512,
    .height = 150,
    .angle = 0.0,
    .zfar = 600
};

void ProcessInput() {
    if (IsKeyDown(KEY_UP)) {
        camera.x += cos(camera.angle);
        camera.y += sin(camera.angle);
    }
    if (IsKeyDown(KEY_DOWN)) {
        camera.x -= cos(camera.angle);
        camera.y -= sin(camera.angle);
    }
    if (IsKeyDown(KEY_LEFT)) {
        camera.angle -= 0.01;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        camera.angle += 0.01;
    }
    if (IsKeyDown(KEY_E)) {
        camera.height++;
    }
    if (IsKeyDown(KEY_D)) {
        camera.height--;
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voxel space rendering");
    
    Image colorMapImage = LoadImage("resources/map0.color.gif");
    Image heightMapImage = LoadImage("resources/map0.height.gif");

    colorMap = LoadImageColors(colorMapImage);
    heightMap = LoadImageColors(heightMapImage);

    SetTargetFPS(FPS);

    while(!WindowShouldClose()) {
        ClearBackground(WHITE);
        ProcessInput();

        float sinangle = sin(camera.angle);
        float cosangle = cos(camera.angle);

        float plx = cosangle * camera.zfar + sinangle * camera.zfar;
        float ply = sinangle * camera.zfar - cosangle * camera.zfar;

        float prx = cosangle * camera.zfar - sinangle * camera.zfar;
        float pry = sinangle * camera.zfar + cosangle * camera.zfar;
 
        BeginDrawing();
            for (size_t i = 0; i < SCREEN_WIDTH; i++) {
                float deltaX = (plx + (prx - plx) / SCREEN_WIDTH * i) / camera.zfar;
                float deltaY = (ply + (pry - ply) / SCREEN_WIDTH * i) / camera.zfar;

                float rx = camera.x;
                float ry = camera.y;

                float maxHeight = SCREEN_HEIGHT;

                for (size_t z = 1; z < camera.zfar; z++) {
                    rx += deltaX;
                    ry -= deltaY;

                    int mapoffset = (MAP_N * ((int)(ry) & 1023)) + ((int)(rx) & 1023);
                    /**printf("%u %u %u ", (int)rx, (int)ry, mapoffset);
                    if (mapoffset < 1024*1024) {
                        printf("True\n");
                    } else {
                        printf("False\n");
                    }**/
                    int heightOnScreen = (int)((camera.height - heightMap[mapoffset].r) / z * SCALE_FACTOR);
                    heightOnScreen = heightOnScreen < 0 ? 0: heightOnScreen;
                    heightOnScreen = heightOnScreen > SCREEN_HEIGHT ? SCREEN_HEIGHT - 1: heightOnScreen;

                    if (heightOnScreen < maxHeight) {
                        for (size_t y = heightOnScreen; y < maxHeight; y++) {
                            DrawPixel(i, y, colorMap[mapoffset]);
                        }
                        maxHeight = heightOnScreen;
                    }
                }
            }    

        EndDrawing();
    }

    CloseWindow();
    return 0;

}
