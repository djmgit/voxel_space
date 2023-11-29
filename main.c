#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <stdio.h>
#include <math.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 400
#define CONTROLS_HEIGHT 200
#define FPS 60
#define MAP_N 1024
#define SCALE_FACTOR 100.0
#define NUM_MAPS 3

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
    float tiltspeed;
    float tilt;
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
    .tiltspeed = 1.5,
    .tilt = 0,
    .zfar = 600
};

typedef struct {
    char colorMap[50];
    char heightMap[50];
} map_t;

map_t maps[NUM_MAPS];

int fogType = 0;
float fogDensity = 0.0025;
float fogStart = 300.0;
float fogEnd = 600.0;
int selectedMap = 0;
int currentSelectedMap = 0;
int mapSelectorMode = 0;

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
    if (IsKeyDown(KEY_Q)) {
        camera.height += camera.heightspeed * timeDelta;
    }
    if (IsKeyDown(KEY_E)) {
        camera.height -= camera.heightspeed * timeDelta;
    }
    if (IsKeyDown(KEY_W)) {
        camera.horizon += camera.horizonspeed * timeDelta;
    }
    if (IsKeyDown(KEY_S)) {
        camera.horizon -= camera.horizonspeed * timeDelta;
    }
    if (IsKeyDown(KEY_A)) {
        camera.tilt -= camera.tiltspeed * timeDelta;
        camera.tilt = camera.tilt < -1 ? -1 : camera.tilt;
    }
    if (IsKeyDown(KEY_D)) {
        camera.tilt += camera.tiltspeed * timeDelta;
        camera.tilt = camera.tilt > 1 ? 1 : camera.tilt;
    }

    if (IsKeyDown(KEY_R)) {
        camera.angle = 1.5 * 3.141592;
        camera.tilt = 0;
        camera.height = 150;
        camera.horizon = 100;
    }
}

int GetLinearFogFactor(int fogEnd, int fogStart, int z) {
    return (int)((fogEnd - z) / (fogEnd - fogStart));
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

void LoadMaps() {
    for (size_t i = 0; i < NUM_MAPS; i++) {
        map_t map;
        sprintf(map.colorMap, "resources/map%d.color.gif", (int)i);
        sprintf(map.heightMap, "resources/map%d.height.gif", (int)i);
        maps[i] = map;
    }
    //maps[0] = (map_t){
        //.colorMap = "resources/map0.color.gif",
        //.heightMap = "resources/map0.height.gif"
    //};
    //maps[1] = (map_t){
        //.colorMap = "resources/map1.color.gif",
        //.heightMap = "resources/map1.height.gif"
    //};
}

int main() {
    LoadMaps();
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voxel space rendering");
    
    Image colorMapImage = LoadImage(maps[selectedMap].colorMap);
    Image heightMapImage = LoadImage(maps[selectedMap].heightMap);

    colorMap = LoadImageColors(colorMapImage);
    heightMap = LoadImageColors(heightMapImage);

    SetTargetFPS(FPS);

    while(!WindowShouldClose()) {
        ClearBackground(RAYWHITE);
        if (currentSelectedMap != selectedMap) {
            selectedMap = currentSelectedMap;
            colorMapImage = LoadImage(maps[selectedMap].colorMap);
            heightMapImage = LoadImage(maps[selectedMap].heightMap);
            colorMap = LoadImageColors(colorMapImage);
            heightMap = LoadImageColors(heightMapImage);
        }
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
                        float lean = (camera.tilt * (i / (float)SCREEN_WIDTH - 0.5) + 0.5) * SCREEN_HEIGHT / 6;

                        for (size_t y = (projHeight + lean); y < (maxHeight + lean); y++) {
                            Color pixel = colorMap[mapoffset];
                            Color scaledPixel = GetScaledPixel(pixel, (Color){180, 180, 180, 255}, GetExponentialFogFactor(fogDensity, z));
                            if (fogType == 1) {
                                if (fogEnd <= fogStart) {
                                    fogEnd = fogStart + 1;
                                }
                                scaledPixel = GetScaledPixel(pixel, (Color){180, 180, 180, 100}, GetLinearFogFactor((int)fogEnd, (int)fogStart, z));
                            }

                            DrawPixel(i, y, scaledPixel);
                        }
                        maxHeight = projHeight;
                    }
                }
            }
            GuiToggleSlider((Rectangle){ 5, 5, 150, 10 }, "Density;Linear", &fogType);
            if (fogType == 0) {
                GuiSliderBar((Rectangle){ 70, 20, 150, 10 }, "Fog Density", TextFormat("%1.4f", fogDensity), &fogDensity, 0.0, 0.02);
            } else {
                GuiSliderBar((Rectangle){ 70, 20, 150, 10 }, "Fog Start", TextFormat("%3.2f", fogStart), &fogStart, 0.0, camera.zfar*1.0);
                GuiSliderBar((Rectangle){ 70, 35, 150, 10 }, "Fog End", TextFormat("%3.2f", fogEnd), &fogEnd, fogStart+1, camera.zfar*1.0);
            }
            if (GuiDropdownBox((Rectangle){ 480, 5, 150, 10 }, "Map0;Map1;Map2", &currentSelectedMap, mapSelectorMode)) mapSelectorMode = !mapSelectorMode;
            //GuiSliderBar((Rectangle){ 100, 500, 200, 20 }, "Fog Density", NULL, &fogDensity, 0.0, 0.02);

        EndDrawing();
    }

    CloseWindow();
    return 0;

}
