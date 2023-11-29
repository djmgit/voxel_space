#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <stdio.h>
#include <math.h>

#define GUI_CONTROLS 1
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 400
#define FPS 60
#define MAP_N 1024
#define SCALE_FACTOR 100.0
#define NUM_MAPS 29

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
    // module for creating fog factor using linear fog
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/fog-formulas
    return (int)((fogEnd - z) / (fogEnd - fogStart));
}

float GetExponentialFogFactor(float fogDensity, int z) {
    // module for creating fog factor applying exponential density
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/fog-formulas
    return (1 / exp(z * fogDensity));
}

Color GetScaledPixel(Color pixel, Color fog, float fogFactor) {
    // Scaling a given fixel with the fog color using our fog factor
    // p = original pixel
    // s = scaled pixel
    // f = fog pixel
    // ff = fog factor
    // s = p*ff + (1-f)*ff
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
}

char* DropdownOptions() {
    char *dropDownText = malloc(NUM_MAPS*6);
    for (size_t i = 0; i < NUM_MAPS; i++) {
        char mapName[6];
        sprintf(mapName, "map%i", (int)i);
        sprintf(dropDownText, "%s;%s", dropDownText, mapName);
    }
    return dropDownText + 1;
}

int main() {
    LoadMaps();
    char *dropDownText = DropdownOptions();
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voxel space");
    
    Image colorMapImage = LoadImage(maps[selectedMap].colorMap);
    Image heightMapImage = LoadImage(maps[selectedMap].heightMap);

    colorMap = LoadImageColors(colorMapImage);
    heightMap = LoadImageColors(heightMapImage);

    SetTargetFPS(FPS);

    while(!WindowShouldClose()) {
        ClearBackground(RAYWHITE);

        // Switch color map and height map if user choses a different map
        if (currentSelectedMap != selectedMap) {
            selectedMap = currentSelectedMap;
            colorMapImage = LoadImage(maps[selectedMap].colorMap);
            heightMapImage = LoadImage(maps[selectedMap].heightMap);
            colorMap = LoadImageColors(colorMapImage);
            heightMap = LoadImageColors(heightMapImage);
        }

        // use time passed since last frame drawn to calculate distances
        // for movements
        float timeDelta = GetFrameTime();
        ProcessInput(timeDelta);

        float sinangle = sin(camera.angle);
        float cosangle = cos(camera.angle);

        /*
        Here we begin with our voxel space rendering algorith. I will try to explain how it works but willa void too much
        details since several good articles online.
        The first thing we need is a camera, we need to decide our field of view (FOV) and the distance we can see in the
        z direction (zfar). 90 degree or pi/2 is a good FOV.

            (plx,ply)       zfar        (prx,pry)
                 \            |            /
                   \          |          /
                     \        |        /
                       \      |      /
                         \    |    /
                           \  |  /
                             \|/
                            (x, y)

        This is how the camera and the extreme left and right casted rays look when the camera is facing up along y axis. In order
        to account for camera rotation we will have to utilise 2d vector rotation.
        Next we divide the distance between the two points of the extreme rays by the width of our screen and then for each
        column we cast a ray. Then along each such ray we go pixel by pixel and fetch the pixels color and height value from the
        color map and the height map. Once we have the pixel and height we draw the pixel at the appropriate position using the
        ray colum as x and height as y.
        Okay we do not use the exact height we are getting from the height map but rather using that height we calculate something
        called the projected height where we also consider the zfar value that is how far looking into the z axis to give it a feel
        of perspective.
        We render the pixels front to back, what this essentially mean is if we find a pixel whose projected height is less than a
        previous pixel, then we dont render the pixel because anywats it wont be visible on the screen. And that is pretty much
        the crux of voxel space algorithm.
        */
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
                    int projHeight = (int)((camera.height - heightMap[mapoffset].r) / z * SCALE_FACTOR + camera.horizon);
                    projHeight = projHeight < 0 ? 0: projHeight;
                    projHeight = projHeight > SCREEN_HEIGHT ? SCREEN_HEIGHT - 1: projHeight;

                    if (projHeight < maxHeight) {
                        float lean = (camera.tilt * (i / (float)SCREEN_WIDTH - 0.5) + 0.5) * SCREEN_HEIGHT / 6;

                        for (size_t y = (projHeight + lean); y < (maxHeight + lean); y++) {
                            Color pixel = colorMap[mapoffset];

                            // Here we scale our pixel to introduce fog.
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
            #ifdef GUI_CONTROLS
                GuiToggleSlider((Rectangle){ 5, 5, 150, 10 }, "Density;Linear", &fogType);
                if (fogType == 0) {
                    GuiSliderBar((Rectangle){ 70, 20, 150, 10 }, "Fog Density", TextFormat("%1.4f", fogDensity), &fogDensity, 0.0, 0.02);
                } else {
                    GuiSliderBar((Rectangle){ 70, 20, 150, 10 }, "Fog Start", TextFormat("%3.2f", fogStart), &fogStart, 0.0, camera.zfar*1.0);
                    GuiSliderBar((Rectangle){ 70, 35, 150, 10 }, "Fog End", TextFormat("%3.2f", fogEnd), &fogEnd, fogStart+1, camera.zfar*1.0);
                }
                if (GuiDropdownBox((Rectangle){ 480, 5, 150, 10 }, dropDownText, &currentSelectedMap, mapSelectorMode)) mapSelectorMode = !mapSelectorMode;
            #endif

        EndDrawing();
    }

    CloseWindow();
    return 0;

}
