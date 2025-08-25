#ifndef MAP_RANDO_H
#define MAP_RANDO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "settings.h"
#include "debug.h"
#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

// Stuff we need elsewhere
extern const int version;
extern const char *suits[];
extern const int suits_size;
extern const char *tileTheme[];
extern const int tile_size;
extern char outputPath[512];

// Settings struct
struct mapRando {
    int skillLevel;
    int itemProgression;
    int qualityOfLife;
    int objectives;
    int mapLayout;
    int doors;
    int startLocation;
    int saveAnimals;
    int wallJumpMode;
    int eTankMode;
    int areaAssignment;
    int dotsFade;
    int doorLock;
    int mapRevealed;
    int mapStation;
    bool freeShinespark;
    bool ultraQuality;
    bool raceMode;
    int roomTheming;
    int doorColors;
    int music;
    int screenShaking;
    int screenFlashing;
    int lowEnergyBeeping;
    int mapArrows;
    int suit;
    int roomPalettes;
    int tileTheme;
    bool roomNames;
    char inputRomPath[256];
    char outputRomPath[256];
    char spoilerToken[256];
};

int generate_map_rando(struct mapRando mapRandoSettings);

#ifdef __cplusplus
}
#endif
#endif