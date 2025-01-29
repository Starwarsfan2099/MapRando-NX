#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdio.h>
#include "map_rando.h"


#ifdef __cplusplus
extern "C" {
#endif

int saveSettingsToFile(struct mapRando *settings, const char *filename);
int loadSettingsFromFile(struct mapRando *settings, const char *filename);

#ifdef __cplusplus
}
#endif
#endif
