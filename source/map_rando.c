#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "settings.h"
#include "debug.h"
#include <switch.h>
#include "map_rando.h"

// Structure to store response data
struct Memory {
    char *response;
    size_t size;
};

// Data we need for sending settings
const int version = 118;
const char *baseUrl = "https://dev.maprando.com";
const char *objectives[] =  {"None", "Bosses", "Minibosses", "Metroids", "Chozos", "Pirates", "Random"};
const char *mapLayout[] = {"Vanilla", "Standard", "Wild"};
const char *doors[] = {"Blue", "Ammo", "Beam"};
const char *startLocation[] = {"Ship", "Random", "Escape"};
const char *saveAnimals[] = {"No", "Yes", "Optional", "Random"};
const char *wallJumpMode[] = {"Vanilla", "Collectible"};
const char *eTankMode[] = {"Disabled", "Vanilla", "Full"};
const char *areaAssignment[] = {"Ordered", "Standard", "Random"};
const char *dotsFade[] = {"Fade", "Disappear"};
const char *mapArrows[] = {"Arrows", "Letters"};
const char *doorLock[] = {"Small", "Large"};
const char *mapRevealed[] = {"No", "Partial", "Full"};
const char *mapStation[] = {"Partial", "Full"};
const char *roomTheming[] = {"Vanilla", "Area Palettes", "Area Tiling", "None"};
const char *doorColors[] = {"vanilla", "alternate"};
const char *music[] = {"area", "disabled"};
const char *screenShaking[] = {"Vanilla", "Reduced", "Disabled"};
const char *screenFlashing[] = {"Vanilla", "Reduced"};
const char *lowEnergyBeeping[] = {"false", "true"};
const char *suits[] = {"samus_vanilla","samus_dread","dread_samus","santamus","metroid_1_suit","samus_fusion_typea_green","dark_samus","dark_samus_2","samus_zero-mission","samus_returns","samus_maid","hack_opposition","ped_suit","ascent","ancient_chozo","super_duper","samus_clocktoberfest","samus_aroace","samus_aroace_2","samus_enby","samus_trans","samus_agender","samus_blue","samus_greyscale","alcoon","alucard_sotn","arcana","bailey","bob","brad_fang","bruno","buffed_kirby","buffed_eggplant","buffed_pug","captain_novolin","ceroba_ketsukane","chairdeep","charizard","charlotte_aran","crest","crewmate","cuphead","cursor","diddy_kong","earthworm_jim","fedtrooper","fight","goku_child","inkling-girl","junko","katt_aran","kiara","kirby","kirby_yarn","knuckles","link_2_the_past","link_oot","link_tall","luigi_mansion","marga","maria_pollo","maria_renard","mario_8bit","mario_8bit_modern","mario_dreamteam","mario_smw","master_hand","maxim_kischine","megamanx","megamanx_bearded","metroid","moonclif","officer_donut","onefourty","plissken","protogen_laso","pyronett","richter_belmont","ronald_mcdonald","samus_combatarmor","sans","shantae","shaktool","shaktool-jr","snes_controller","sonic","space_pirate","sprite_can","super_controid_pg","tails","tetris","thomcrow_corbin","wario","yoshi","zero_suit_samus","samus_outline","hitboxhelper2","samus_backwards","samus_upsidedown","samus_180-degree","samus_mini","samus_left-leg","samus_cannon","samus_invisible"};
const int suits_size = sizeof(suits) / sizeof(suits[0]);
const char *roomPalettes[] = {"vanilla", "area-themed"};
const char *tileTheme[] = {"none","area_themed","scrambled","OuterCrateria","InnerCrateria","BlueBrinstar","GreenBrinstar","PinkBrinstar","RedBrinstar","UpperNorfair","LowerNorfair","WreckedShip","WestMaridia","YellowMaridia","MechaTourian","MetroidHabitat","Outline","Invisible"};
const int tile_size = sizeof(tileTheme) / sizeof(tileTheme[0]);
char outputPath[512];
const char *userAgent = "Switch Homebrew - MapRando-NX";

// Callback function for handling response data
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t totalSize = size * nmemb;
    struct Memory *mem = (struct Memory *)userp;

    char *ptr = realloc(mem->response, mem->size + totalSize + 1);
    if (ptr == NULL) {
        TRACE("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), contents, totalSize);
    mem->size += totalSize;
    mem->response[mem->size] = '\0';

    return totalSize;
}

// Function to send the first request
char *send_request_1(const char *file_path, const char *spoiler_token, const char *settings) {
    CURL *curl;
    CURLcode res;
    struct Memory chunk = {NULL, 0};
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    curl = curl_easy_init();
    if (!curl) {
        TRACE("Failed to initialize CURL\n");
        return NULL;
    }

    // Add User Agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);

    // Build form data
    curl_formadd(&formpost, &lastptr,
                    CURLFORM_COPYNAME, "spoiler_token",
                    CURLFORM_COPYCONTENTS, spoiler_token,
                    CURLFORM_END);

    curl_formadd(&formpost, &lastptr,
                    CURLFORM_COPYNAME, "settings",
                    CURLFORM_COPYCONTENTS, settings,
                    CURLFORM_END);

    curl_formadd(&formpost, &lastptr,
                    CURLFORM_COPYNAME, "rom",
                    CURLFORM_FILE, file_path,
                    CURLFORM_CONTENTTYPE, "application/octet-stream",
                    CURLFORM_END);

    // Set options
    char customize_url[512];
    snprintf(customize_url, sizeof(customize_url), "%s/randomize", baseUrl);
    curl_easy_setopt(curl, CURLOPT_URL, customize_url);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Perform the request
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        TRACE("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_formfree(formpost);
        return NULL;
    }

    // Clean up
    curl_easy_cleanup(curl);
    curl_formfree(formpost);
    curl_global_cleanup();

    return chunk.response;
}

// Function to send the second request
int send_request_2(const char *seedUrl, const char *file_path, const char *output_path, struct mapRando mapRandoSettings) {
    CURL *curl;
    CURLcode res;
    FILE *output_file;
    long response_code;

    output_file = fopen(output_path, "wb");
    if (!output_file) {
        TRACE("Failed to open output file");
        return -1;
    }

    // Initialize CURL
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl) {
        TRACE("Failed to initialize CURL\n");
        fclose(output_file);
        return -1;
    }

    // Create the multipart/form-data request
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;

    // Add User Agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);

    // Add the file
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "rom",
                 CURLFORM_FILE, file_path,
                 CURLFORM_CONTENTTYPE, "application/octet-stream",
                 CURLFORM_FILENAME, "Super Metroid (JU) [!].smc",
                 CURLFORM_END);

    // Add more form data
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "samus_sprite", CURLFORM_COPYCONTENTS, suits[mapRandoSettings.suit], CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "etank_color", CURLFORM_COPYCONTENTS, "96de38", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "energy_tank_color", CURLFORM_COPYCONTENTS, "", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "room_theming", CURLFORM_COPYCONTENTS, roomTheming[mapRandoSettings.roomTheming], CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "door_theme", CURLFORM_COPYCONTENTS, doorColors[mapRandoSettings.doorColors], CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "music", CURLFORM_COPYCONTENTS, music[mapRandoSettings.music], CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "shaking", CURLFORM_COPYCONTENTS, screenShaking[mapRandoSettings.screenShaking], CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "flashing", CURLFORM_COPYCONTENTS, screenFlashing[mapRandoSettings.screenFlashing], CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "disable_beeping", CURLFORM_COPYCONTENTS, lowEnergyBeeping[mapRandoSettings.lowEnergyBeeping], CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "room_palettes", CURLFORM_COPYCONTENTS, roomPalettes[mapRandoSettings.roomPalettes], CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "tile_theme", CURLFORM_COPYCONTENTS, tileTheme[mapRandoSettings.tileTheme], CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "reserve_hud_style", CURLFORM_COPYCONTENTS, "true", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "vanilla_screw_attack_animation", CURLFORM_COPYCONTENTS, "false", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "control_shot", CURLFORM_COPYCONTENTS, "X", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "control_jump", CURLFORM_COPYCONTENTS, "A", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "control_dash", CURLFORM_COPYCONTENTS, "B", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "control_item_select", CURLFORM_COPYCONTENTS, "Select", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "control_item_cancel", CURLFORM_COPYCONTENTS, "Y", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "control_angle_up", CURLFORM_COPYCONTENTS, "R", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "control_angle_down", CURLFORM_COPYCONTENTS, "L", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "quick_reload_l", CURLFORM_COPYCONTENTS, "on", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "quick_reload_r", CURLFORM_COPYCONTENTS, "on", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "quick_reload_select", CURLFORM_COPYCONTENTS, "on", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "quick_reload_start", CURLFORM_COPYCONTENTS, "on", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "spin_lock_x", CURLFORM_COPYCONTENTS, "on", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "spin_lock_l", CURLFORM_COPYCONTENTS, "on", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "spin_lock_r", CURLFORM_COPYCONTENTS, "on", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "spin_lock_up", CURLFORM_COPYCONTENTS, "on", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "moonwalk", CURLFORM_COPYCONTENTS, "true", CURLFORM_END);

    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, seedUrl);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, output_file);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);

    // Perform the request
    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    fclose(output_file);

    // Debugging output
    if (res != CURLE_OK) {
        TRACE("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_formfree(formpost);
        curl_easy_cleanup(curl);
        return -1;
    }

    if (response_code != 200) {
        TRACE("Server returned status code: %ld\n", response_code);
        remove(output_path); // Remove incomplete file
        curl_formfree(formpost);
        curl_easy_cleanup(curl);
        return -1;
    }

    // Clean up
    curl_formfree(formpost);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}

// Function to extract the seed_url value from the JSON response
char *extract_seed_url(const char *response) {
    const char *key = "\"seed_url\":\"";
    char *start = strstr(response, key);
    if (!start) {
        TRACE("Error: seed_url not found in response.\n");
        return NULL;
    }

    start += strlen(key);
    char *end = strchr(start, '"');
    if (!end) {
        TRACE("Error: Invalid JSON format.\n");
        return NULL;
    }

    size_t length = end - start;
    char *seedUrl = (char *)malloc(length + 1);
    if (!seedUrl) {
        TRACE("Failed to allocate memory");
        return NULL;
    }

    strncpy(seedUrl, start, length);
    seedUrl[length] = '\0'; // Null-terminate the string
    return seedUrl;
}

int generate_map_rando(struct mapRando mapRandoSettings) {
    TRACE("Generating...");

    struct json_object *skill_preset = json_tokener_parse(skillPresetsArr[mapRandoSettings.skillLevel + 1]);
    if (skill_preset == NULL) {
        TRACE("Failed to parse skill_preset JSON string.\n");
        return 1;
    }

    struct json_object *item_presets = json_tokener_parse(itemPresetsArr[mapRandoSettings.itemProgression]);
    if (item_presets == NULL) {
        TRACE("Failed to parse item_presets JSON string.\n");
        return 1;
    }

    struct json_object *qol_presets = json_tokener_parse(qolPresetsArr[mapRandoSettings.qualityOfLife]);
    if (qol_presets == NULL) {
        TRACE("Failed to parse qol_presets JSON string.\n");
        return 1;
    }

    struct json_object *other_settings = json_tokener_parse(otherSettings);
    if (other_settings == NULL) {
        TRACE("Failed to parse other_settings JSON string.\n");
        return 1;
    }

    // Other settings
    struct json_object *wallJumpObject = json_object_new_string(wallJumpMode[mapRandoSettings.wallJumpMode]);
    json_object_object_add(other_settings, "wall_jump", wallJumpObject);
    struct json_object *eTankObject = json_object_new_string(eTankMode[mapRandoSettings.eTankMode]);
    json_object_object_add(other_settings, "etank_refill", eTankObject);
    struct json_object *areaAssignmentObject = json_object_new_string(areaAssignment[mapRandoSettings.areaAssignment]);
    json_object_object_add(other_settings, "area_assignment", areaAssignmentObject);
    struct json_object *dotsFadeObject = json_object_new_string(dotsFade[mapRandoSettings.dotsFade]);
    json_object_object_add(other_settings, "item_dot_change", dotsFadeObject);
    struct json_object *doorLockObject = json_object_new_string(doorLock[mapRandoSettings.doorLock]);
    json_object_object_add(other_settings, "door_locks_size", doorLockObject);
    struct json_object *mapRevealedObject = json_object_new_string(mapRevealed[mapRandoSettings.mapRevealed]);
    json_object_object_add(other_settings, "maps_revealed", mapRevealedObject);
    struct json_object *mapStationObject = json_object_new_string(mapStation[mapRandoSettings.mapStation]);
    json_object_object_add(other_settings, "map_station_reveal", mapStationObject);
    json_object_object_add(other_settings, "energy_free_shinesparks", json_object_new_boolean(mapRandoSettings.freeShinespark));
    json_object_object_add(other_settings, "ultra_low_qol", json_object_new_boolean(mapRandoSettings.ultraQuality));
    json_object_object_add(other_settings, "race_mode", json_object_new_boolean(mapRandoSettings.raceMode));

    // Build the json settings object
    struct json_object *main_obj = json_object_new_object();
    json_object_object_add(main_obj, "version", json_object_new_int(version));
    json_object_object_add(main_obj, "name", json_object_new_string(""));
    json_object_object_add(main_obj, "skill_assumption_settings", skill_preset);
    json_object_object_add(main_obj, "item_progression_settings", item_presets);
    json_object_object_add(main_obj, "quality_of_life_settings", qol_presets);
    json_object_object_add(main_obj, "objectives_mode", json_object_new_string(objectives[mapRandoSettings.objectives]));
    json_object_object_add(main_obj, "map_layout", json_object_new_string(mapLayout[mapRandoSettings.mapLayout]));
    json_object_object_add(main_obj, "doors_mode", json_object_new_string(doors[mapRandoSettings.doors]));
    json_object_object_add(main_obj, "start_location_mode", json_object_new_string(startLocation[mapRandoSettings.startLocation]));
    json_object_object_add(main_obj, "save_animals", json_object_new_string(saveAnimals[mapRandoSettings.saveAnimals]));
    json_object_object_add(main_obj, "other_settings", other_settings);
    //TRACE("Resulting JSON:\n%s\n", json_object_to_json_string_ext(main_obj, JSON_C_TO_STRING_PRETTY));

    // Send the first request
    char *seedUrl = extract_seed_url(send_request_1(mapRandoSettings.inputRomPath, mapRandoSettings.spoilerToken, json_object_to_json_string_ext(main_obj, JSON_C_TO_STRING_PLAIN)));
    if (!seedUrl) {
        TRACE("Failed to send the first request.\n");
        return 1;
    }

    TRACE("Seed URL: %s\n", seedUrl);
    consoleUpdate(NULL);

    // Construct the customization URL
    char customize_url[512];
    snprintf(customize_url, sizeof(customize_url), "%s%scustomize", baseUrl, seedUrl);
    TRACE("%s", customize_url);

    char *seedPart = strstr(seedUrl, "/seed/");
    if (seedPart != NULL) {
        seedPart += strlen("/seed/");
        size_t len = strlen(seedPart);
        if (len > 0 && seedPart[len - 1] == '/') {
            seedPart[len - 1] = '\0';  // Remove the last character
        }
        char finalPath[256];
        snprintf(finalPath, sizeof(finalPath), "/map-rando-%s", seedPart);
        snprintf(outputPath, sizeof(outputPath), "%s%s.sfc", mapRandoSettings.outputRomPath, finalPath);
        TRACE("Combined Path: %s\n", outputPath);
    } else {
        TRACE("No valid seed part found.\n");
    }

    // Send the second request
    if (send_request_2(customize_url, mapRandoSettings.inputRomPath, outputPath, mapRandoSettings) != 0) {
        TRACE("Failed to send the second request.\n");
        return 1;
    }

    TRACE("Customized ROM saved as %s\n", outputPath);
    return 0;
}
