#include "raylib.h"
#include <stdio.h>
#include <math.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 400
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
    float horizon;
    float speed;
    float rotspeed;
    float heightspeed;
    float horizonspeed;
    float zfar;
} camera_t;

camera_t camera = {
    .x = 512,
    .y = 512,
    .height = 150,
    .angle = 1.5 * 3.141592,
    .horizon = 100,
    .speed = 60,
    .rotspeed = 0.5,
    .heightspeed = 100,
    .horizonspeed = 100,
    .zfar = 600
};

void ProcessInput(float timeDelta) {
    if (IsKeyDown(KEY_UP)) {
        camera.x += camera.speed * cos(camera.angle) * timeDelta;
        camera.y += camera.speed * sin(camera.angle) * timeDelta;
    }
    if (IsKeyDown(KEY_DOWN)) {
        camera.x -= camera.speed * cos(camera.angle) * timeDelta;
        camera.y -= camera.speed * sin(camera.angle) * timeDelta;
    }
    if (IsKeyDown(KEY_LEFT)) {
        camera.angle -= camera.rotspeed * timeDelta;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        camera.angle += camera.rotspeed * timeDelta;
    }
    if (IsKeyDown(KEY_E)) {
        camera.height += camera.heightspeed * timeDelta;
    }
    if (IsKeyDown(KEY_D)) {
        camera.height -= camera.heightspeed * timeDelta;
    }
    if (IsKeyDown(KEY_Q)) {
        camera.horizon += camera.horizonspeed * timeDelta;
    }
    if (IsKeyDown(KEY_W)) {
        camera.horizon -= camera.horizonspeed * timeDelta;
    }
}

float GetLinearFogFactor(int fogEnd, int fogStart, int z) {
    return (fogEnd - z) / (fogEnd - fogStart);
}

float GetExponentialFogFactor(float fogDensity, int z) {
    return (1 / exp(z * fogDensity));
}

Color GetScaledPixel(Color pixel, Color fog, float fogFactor) {
    pixel.r = pixel.r * fogFactor;
    pixel.g = pixel.g * fogFactor;
    pixel.b = pixel.b * fogFactor;
    pixel.a = pixel.a * fogFactor;
    fog.r = fog.r * (1 - fogFactor);
    fog.g = fog.g * (1 - fogFactor);
    fog.b = fog.b * (1 - fogFactor);
    fog.a = fog.a * (1 - fogFactor);
    pixel.r = pixel.r + fog.r;
    pixel.g = pixel.g + fog.g;
    pixel.b = pixel.b + fog.b;
    pixel.a = pixel.a + fog.a;

    return pixel;
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voxel space rendering");
    
    Image colorMapImage = LoadImage("resources/map0.color.gif");
    Image heightMapImage = LoadImage("resources/map0.height.gif");

    colorMap = LoadImageColors(colorMapImage);
    heightMap = LoadImageColors(heightMapImage);

    SetTargetFPS(FPS);

    while(!WindowShouldClose()) {
        ClearBackground(RAYWHITE);
        float timeDelta = GetFrameTime();
        ProcessInput(timeDelta);

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
                    ry += deltaY;

                    int mapoffset = (MAP_N * ((int)(ry) & (MAP_N - 1))) + ((int)(rx) & (MAP_N - 1));
                    /**printf("%u %u %u ", (int)rx, (int)ry, mapoffset);
                    if (mapoffset < 1024*1024) {
                        printf("True\n");
                    } else {
                        printf("False\n");
                    }**/
                    int projHeight = (int)((camera.height - heightMap[mapoffset].r) / z * SCALE_FACTOR + camera.horizon);
                    projHeight = projHeight < 0 ? 0: projHeight;
                    projHeight = projHeight > SCREEN_HEIGHT ? SCREEN_HEIGHT - 1: projHeight;

                    if (projHeight < maxHeight) {
                        for (size_t y = projHeight; y < maxHeight; y++) {
                            Color pixel = colorMap[mapoffset];
                            //Color scaledPixel = GetScaledPixel(pixel, (Color){180, 180, 180, 100}, GetLinearFogFactor(600, 300, z));
                            Color scaledPixel = GetScaledPixel(pixel, (Color){180, 180, 180, 255}, GetExponentialFogFactor(0.0025, z));

                            DrawPixel(i, y, scaledPixel);
                        }
                        maxHeight = projHeight;
                    }
                }
            }    

        EndDrawing();
    }

    CloseWindow();
    return 0;

}
