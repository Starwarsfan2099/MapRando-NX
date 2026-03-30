#include "config.h"
#include <string.h>
#include "map_rando.h"
#include "debug.h"

#include <stdlib.h>

static void copy_config_string(char *dst, size_t dst_size, const char *src)
{
    size_t len = strnlen(src, dst_size - 1);
    memcpy(dst, src, len);
    dst[len] = '\0';
}


int saveSettingsToFile(struct mapRando *settings, const char *filename) {
    TRACE("Saving settings...");
    FILE *file = fopen(filename, "w");
    if (!file) {
        return -1; // Error opening file
    }

    // Write settings as plain text
    fprintf(file,
            "skillLevel=%d\n"
            "preset=%d\n"
            "itemProgression=%d\n"
            "qualityOfLife=%d\n"
            "objectives=%d\n"
            "mapLayout=%d\n"
            "doors=%d\n"
            "startLocation=%d\n"
            "saveAnimals=%d\n"
            "wallJumpMode=%d\n"
            "eTankMode=%d\n"
            "areaAssignment=%d\n"
            "dotsFade=%d\n"
            "doorLock=%d\n"
            "mapRevealed=%d\n"
            "mapStation=%d\n"
            "freeShinespark=%d\n"
            "ultraQuality=%d\n"
            "raceMode=%d\n"
            "roomTheming=%d\n"
            "doorColors=%d\n"
            "music=%d\n"
            "screenShaking=%d\n"
            "screenFlashing=%d\n"
            "lowEnergyBeeping=%d\n"
            "mapArrows=%d\n"
            "suit=%d\n"
            "roomPalettes=%d\n"
            "tileTheme=%d\n"
            "roomNames=%d\n"
            "bossIcons=%d\n"
            "minibossIcons=%d\n"
            "saveIcons=%d\n"
            "speedBoosterSplit=%d\n"
            "statuesHallwayTiling=%d\n"
            "statuesHallwayAudio=%d\n"
            "inputRomPath=%s\n"
            "outputRomPath=%s\n"
            "spoilerToken=%s\n",
            settings->skillLevel,
            settings->preset,
            settings->itemProgression,
            settings->qualityOfLife,
            settings->objectives,
            settings->mapLayout,
            settings->doors,
            settings->startLocation,
            settings->saveAnimals,
            settings->wallJumpMode,
            settings->eTankMode,
            settings->areaAssignment,
            settings->dotsFade,
            settings->doorLock,
            settings->mapRevealed,
            settings->mapStation,
            settings->freeShinespark,
            settings->ultraQuality,
            settings->raceMode,
            settings->roomTheming,
            settings->doorColors,
            settings->music,
            settings->screenShaking,
            settings->screenFlashing,
            settings->lowEnergyBeeping,
            settings->mapArrows,
            settings->suit,
            settings->roomPalettes,
            settings->tileTheme,
            settings->roomNames,
            settings->bossIcons,
            settings->minibossIcons,
            settings->saveIcons,
            settings->speedBoosterSplit,
            settings->statuesHallwayTiling,
            settings->statuesHallwayAudio,
            settings->inputRomPath,
            settings->outputRomPath,
            settings->spoilerToken);

    fclose(file);
    return 0; // Success
}

int loadSettingsFromFile(struct mapRando *settings, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1; // Error opening file
    }

    char buffer[512]; // Temporary buffer for reading lines
    while (fgets(buffer, sizeof(buffer), file)) {
        // Parse each line
        if (sscanf(buffer, "skillLevel=%d", &settings->skillLevel) == 1) continue;
        if (sscanf(buffer, "preset=%d", &settings->preset) == 1) continue;
        if (sscanf(buffer, "itemProgression=%d", &settings->itemProgression) == 1) continue;
        if (sscanf(buffer, "qualityOfLife=%d", &settings->qualityOfLife) == 1) continue;
        if (sscanf(buffer, "objectives=%d", &settings->objectives) == 1) continue;
        if (sscanf(buffer, "mapLayout=%d", &settings->mapLayout) == 1) continue;
        if (sscanf(buffer, "doors=%d", &settings->doors) == 1) continue;
        if (sscanf(buffer, "startLocation=%d", &settings->startLocation) == 1) continue;
        if (sscanf(buffer, "saveAnimals=%d", &settings->saveAnimals) == 1) continue;
        if (sscanf(buffer, "wallJumpMode=%d", &settings->wallJumpMode) == 1) continue;
        if (sscanf(buffer, "eTankMode=%d", &settings->eTankMode) == 1) continue;
        if (sscanf(buffer, "areaAssignment=%d", &settings->areaAssignment) == 1) continue;
        if (sscanf(buffer, "dotsFade=%d", &settings->dotsFade) == 1) continue;
        if (sscanf(buffer, "doorLock=%d", &settings->doorLock) == 1) continue;
        if (sscanf(buffer, "mapRevealed=%d", &settings->mapRevealed) == 1) continue;
        if (sscanf(buffer, "mapStation=%d", &settings->mapStation) == 1) continue;
        int temp;
        if (sscanf(buffer, "freeShinespark=%d", &temp) == 1) { settings->freeShinespark = (bool)temp; continue; }
        if (sscanf(buffer, "ultraQuality=%d", &temp) == 1) { settings->ultraQuality = (bool)temp; continue; }
        if (sscanf(buffer, "raceMode=%d", &temp) == 1) { settings->raceMode = (bool)temp; continue; }
        if (sscanf(buffer, "roomTheming=%d", &settings->roomTheming) == 1) continue;
        if (sscanf(buffer, "doorColors=%d", &settings->doorColors) == 1) continue;
        if (sscanf(buffer, "music=%d", &settings->music) == 1) continue;
        if (sscanf(buffer, "screenShaking=%d", &settings->screenShaking) == 1) continue;
        if (sscanf(buffer, "screenFlashing=%d", &settings->screenFlashing) == 1) continue;
        if (sscanf(buffer, "lowEnergyBeeping=%d", &settings->lowEnergyBeeping) == 1) continue;
        if (sscanf(buffer, "mapArrows=%d", &settings->mapArrows) == 1) continue;
        if (sscanf(buffer, "suit=%d", &settings->suit) == 1) continue;
        if (sscanf(buffer, "roomPalettes=%d", &settings->roomPalettes) == 1) continue;
        if (sscanf(buffer, "tileTheme=%d", &settings->tileTheme) == 1) continue;
        if (sscanf(buffer, "roomNames=%d", &temp) == 1) { settings->roomNames = (bool)temp; continue; }
        if (sscanf(buffer, "bossIcons=%d", &temp) == 1) { settings->bossIcons = (bool)temp; continue; }
        if (sscanf(buffer, "minibossIcons=%d", &temp) == 1) { settings->minibossIcons = (bool)temp; continue; }
        if (sscanf(buffer, "saveIcons=%d", &temp) == 1) { settings->saveIcons = (bool)temp; continue; }
        if (sscanf(buffer, "speedBoosterSplit=%d", &temp) == 1) { settings->speedBoosterSplit = (bool)temp; continue; }
        if (sscanf(buffer, "statuesHallwayTiling=%d", &settings->statuesHallwayTiling) == 1) continue;
        if (sscanf(buffer, "statuesHallwayAudio=%d", &settings->statuesHallwayAudio) == 1) continue;

        // Handle strings with spaces
        if (strncmp(buffer, "inputRomPath=", 13) == 0) {
            copy_config_string(settings->inputRomPath,
                            sizeof(settings->inputRomPath),
                            buffer + 13);
            settings->inputRomPath[strcspn(settings->inputRomPath, "\n")] = '\0';
            continue;
        }

        if (strncmp(buffer, "outputRomPath=", 14) == 0) {
            copy_config_string(settings->outputRomPath,
                            sizeof(settings->outputRomPath),
                            buffer + 14);
            settings->outputRomPath[strcspn(settings->outputRomPath, "\n")] = '\0';
            continue;
        }

        if (strncmp(buffer, "spoilerToken=", 13) == 0) {
            copy_config_string(settings->spoilerToken,
                            sizeof(settings->spoilerToken),
                            buffer + 13);
            settings->spoilerToken[strcspn(settings->spoilerToken, "\n")] = '\0';
            continue;
        }

    }

    fclose(file);
    return 0; // Success
}
