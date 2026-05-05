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
const int version = 121;
const char *baseUrl = "https://maprando.com";
const char *objectives[] =  {"None", "Bosses", "Minibosses", "Metroids", "Chozos", "Pirates", "Random"};
const char *mapLayout[] = {"Vanilla", "Small", "Standard", "Wild"};
const char *doors[] = {"Blue", "Ammo", "Beam"};
const char *startLocation[] = {"Ship", "Random", "Escape"};
const char *saveAnimals[] = {"No", "Yes", "Optional", "Random"};
const char *wallJumpMode[] = {"Vanilla", "Collectible"};
const char *eTankMode[] = {"Disabled", "Vanilla", "Full"};
const char *areaAssignment[] = {"Standard", "Size", "Depth", "Random"};
const char *dotsFade[] = {"Fade", "Disappear"};
const char *mapArrows[] = {"Arrows", "Letters"};
const char *doorLock[] = {"Small", "Large"};
const char *mapRevealed[] = {"No", "Partial", "Full"};
const char *mapStation[] = {"Partial", "Full"};
const char *roomTheming[] = {"vanilla", "palettes", "tiling", ""};
const char *doorColors[] = {"vanilla", "alternate"};
const char *music[] = {"area", "disabled"};
const char *screenShaking[] = {"Vanilla", "Reduced", "Disabled"};
const char *screenFlashing[] = {"Vanilla", "Reduced"};
const char *lowEnergyBeeping[] = {"false", "true"};
const char *statuesHallwayOptions[] = {"Disabled", "Default", "Enabled"};
const char *statuesHallwayAudioOptions[] = {"Disabled", "Enabled", "Louder"};
const char *suits[] = {"samus_vanilla","metroid_1_suit","samus_zero-mission","samus_returns","samus_fusion_typea_green","prime_series_suit","ped_suit","samus_dread","dread_samus","metroid_suit","ascent","ancient_chozo_pg","super_duper","hack_opposition","samus_aroace","samus_aroace_2","samus_enby","samus_trans","samus_agender","dark_samus","dark_samus_2","dark_samus_reanimated","samus_maid","santamus","samus_blue","bastion","samus_clocktoberfest","samus_greyscale","samus_outline","alcoon","alucard_sotn","arcana","bailey","bart_simpson","bob","brad_fang","bruno","buffed_kirby","buffed_eggplant","buffed_pug","cacodemon","captain_novolin","ceroba_ketsukane","chairdeep","charizard","charlotte_aran","crest","crewmate","cuphead","cursor","diddy_kong","earthworm_jim","elista","fedtrooper","fight","goku_child","green_mm","infee_nitee","inkling-girl","junko","katt_aran","kiara","kiara_idol","king_of_pop","kirby","kirby_yarn","knuckles","link_2_the_past","link_oot","link_tall","luigi_mansion","lyn","maddie_and_baddie","marga","maria_pollo","maria_renard","mario_8bit","mario_8bit_modern","mario_dreamteam","mario_smw","master_hand","maxim_kischine","megamanx","megamanx_bearded","metroid","modul","moonclif","officer_donut","onefourty","plissken","protogen_laso","pyronett","pyronett_a","richter_belmont","alien_3_ripley","ronald_mcdonald","samus_combatarmor","sans","shantae","shaktool","shaktool-jr","snes_controller","sonic","advance_sonic","space_pirate","spider_man","spongebob","sprite_can","super_controid_pg","tails","tetris","terrifier","thomcrow_corbin","V1","wario","yoshi","zero_suit_samus","samus_backwards","samus_upsidedown","samus_180-degree","samus_mini","samus_left-leg","samus_cannon","samus_invisible","hitboxhelper2","magic_pants"};
const int suits_size = sizeof(suits) / sizeof(suits[0]);
const char *roomPalettes[] = {"vanilla", "area_themed"};
const char *tileTheme[] = {"none","area_themed","scrambled","OuterCrateria","InnerCrateria","BlueBrinstar","GreenBrinstar","PinkBrinstar","RedBrinstar","WarehouseBrinstar","UpperNorfair","LowerNorfair","WreckedShip","WestMaridia","YellowMaridia","Bedrock","MechaTourian","MetroidHabitat","StatuesHallway","Outline","Invisible"};
const char *presets[] = {"None","Default","Community Race Season 5","Mentor Tournament","Summer Series Expert Challenge"};
const char *mapThemes[] = {"Dark", "Light"};
const int tile_size = sizeof(tileTheme) / sizeof(tileTheme[0]);
const int presets_size = sizeof(presets) / sizeof(presets[0]);
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
        free(chunk.response);
        curl_easy_cleanup(curl);
        curl_formfree(formpost);
        return NULL;
    }

    // Clean up
    curl_easy_cleanup(curl);
    curl_formfree(formpost);

    return chunk.response;
}

// Function to send the second request
int send_request_2(const char *seedUrl, const char *file_path, const char *output_path, struct mapRando mapRandoSettings) {
    CURL *curl;
    CURLcode res;
    FILE *output_file;
    long response_code;
    const char* roomNames = mapRandoSettings.roomNames ? "true" : "false";
    const char* bossIcons = mapRandoSettings.bossIcons ? "true" : "false";
    const char* minibossIcons = mapRandoSettings.minibossIcons ? "true" : "false";
    const char* saveIcons = mapRandoSettings.saveIcons ? "true" : "false";
    const char* transitionLetters = strcmp(mapArrows[mapRandoSettings.mapArrows], "Letters") == 0 ? "true" : "false";

    output_file = fopen(output_path, "wb");
    if (!output_file) {
        TRACE("Failed to open output file");
        return -1;
    }

    // Initialize CURL
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
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "statues_hallway_tiling", CURLFORM_COPYCONTENTS, statuesHallwayOptions[mapRandoSettings.statuesHallwayTiling], CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "statues_hallway_audio", CURLFORM_COPYCONTENTS, statuesHallwayAudioOptions[mapRandoSettings.statuesHallwayAudio], CURLFORM_END);
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
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "room_names", CURLFORM_COPYCONTENTS, roomNames, CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "boss_icons", CURLFORM_COPYCONTENTS, bossIcons, CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "miniboss_icons", CURLFORM_COPYCONTENTS, minibossIcons, CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "save_icons", CURLFORM_COPYCONTENTS, saveIcons, CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "transition_letters", CURLFORM_COPYCONTENTS, transitionLetters, CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "item_dot_change", CURLFORM_COPYCONTENTS, dotsFade[mapRandoSettings.dotsFade], CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "map_theme", CURLFORM_COPYCONTENTS, mapThemes[mapRandoSettings.mapTheme], CURLFORM_END);

    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, seedUrl);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, output_file);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

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

        // Print error contents
        FILE *f = fopen(output_path, "rb");
        if (f) {
            char buf[128];
            size_t n;
            TRACE("Server response body:\n");
            while ((n = fread(buf, 1, sizeof(buf) - 1, f)) > 0) {
                buf[n] = '\0';
                TRACE("%s", buf);
            }
            fclose(f);
        }

        remove(output_path); // Remove incomplete file
        curl_formfree(formpost);
        curl_easy_cleanup(curl);
        return -1;
    }

    // Clean up
    curl_formfree(formpost);
    curl_easy_cleanup(curl);

    return 0;
}

// Function to extract the seed_url value from the JSON response
char *extract_seed_url(const char *response) {
    struct json_object *root = json_tokener_parse(response);
    if (!root) {
        TRACE("Error: Failed to parse JSON response.\n");
        return NULL;
    }

    struct json_object *seed_url_obj;
    if (!json_object_object_get_ex(root, "seed_url", &seed_url_obj)) {
        TRACE("Error: seed_url not found in response.\n");
        json_object_put(root);
        return NULL;
    }

    const char *seed_url_str = json_object_get_string(seed_url_obj);
    if (!seed_url_str) {
        TRACE("Error: seed_url is not a string.\n");
        json_object_put(root);
        return NULL;
    }

    char *seedUrl = strdup(seed_url_str);
    json_object_put(root);
    return seedUrl;
}

int generate_map_rando(struct mapRando mapRandoSettings) {
    const char* speedBoosterSplit = mapRandoSettings.speedBoosterSplit ? "Split" : "Vanilla";
    TRACE("Generating...");

    struct json_object *main_obj = NULL;
    struct json_object *skill_preset = NULL;
    struct json_object *item_presets = NULL;
    struct json_object *qol_presets = NULL;
    struct json_object *other_settings_obj = NULL;
    struct json_object *area_assignment_obj = NULL;

    if (mapRandoSettings.preset != 0) {
        // Use predefined json for the presets
        TRACE("Using preset...");
        main_obj = json_tokener_parse(fullPresetsArr[mapRandoSettings.preset - 1]);
        if (main_obj == NULL) {
            TRACE("Failed to parse full preset JSON.\n");
            return 1;
        }
    } else {
        // Other settings
        TRACE("Not using preset...");
        
        skill_preset = json_tokener_parse(skillPresetsArr[mapRandoSettings.skillLevel + 1]);
        if (skill_preset == NULL) {
            TRACE("Failed to parse skill_preset JSON string.\n");
            return 1;
        }

        item_presets = json_tokener_parse(itemPresetsArr[mapRandoSettings.itemProgression]);
        if (item_presets == NULL) {
            TRACE("Failed to parse item_presets JSON string.\n");
            json_object_put(skill_preset);
            return 1;
        }

        qol_presets = json_tokener_parse(qolPresetsArr[mapRandoSettings.qualityOfLife]);
        if (qol_presets == NULL) {
            TRACE("Failed to parse qol_presets JSON string.\n");
            json_object_put(skill_preset);
            json_object_put(item_presets);
            return 1;
        }

        other_settings_obj = json_tokener_parse(otherSettings);
        if (other_settings_obj == NULL) {
            TRACE("Failed to parse other_settings JSON string.\n");
            json_object_put(skill_preset);
            json_object_put(item_presets);
            json_object_put(qol_presets);
            return 1;
        }

        json_object_object_add(other_settings_obj, "wall_jump", json_object_new_string(wallJumpMode[mapRandoSettings.wallJumpMode]));
        json_object_object_add(other_settings_obj, "etank_refill", json_object_new_string(eTankMode[mapRandoSettings.eTankMode]));
        if (json_object_object_get_ex(other_settings_obj, "area_assignment", &area_assignment_obj)) {
            // Standard
            if (mapRandoSettings.areaAssignment == 0) {
                json_object_object_add(area_assignment_obj, "preset", json_object_new_string(areaAssignment[mapRandoSettings.areaAssignment]));
                json_object_object_add(area_assignment_obj, "base_order", json_object_new_string("Size"));
            }
            // Size, Depth, and Random
            else {
                json_object_object_add(area_assignment_obj, "preset", json_object_new_string(areaAssignment[mapRandoSettings.areaAssignment]));
                json_object_object_add(area_assignment_obj, "base_order", json_object_new_string(areaAssignment[mapRandoSettings.areaAssignment]));
            }
        }
        json_object_object_add(other_settings_obj, "door_locks_size", json_object_new_string(doorLock[mapRandoSettings.doorLock]));
        json_object_object_add(other_settings_obj, "maps_revealed", json_object_new_string(mapRevealed[mapRandoSettings.mapRevealed]));
        json_object_object_add(other_settings_obj, "map_station_reveal", json_object_new_string(mapStation[mapRandoSettings.mapStation]));
        json_object_object_add(other_settings_obj, "energy_free_shinesparks", json_object_new_boolean(mapRandoSettings.freeShinespark));
        json_object_object_add(other_settings_obj, "ultra_low_qol", json_object_new_boolean(mapRandoSettings.ultraQuality));
        json_object_object_add(other_settings_obj, "race_mode", json_object_new_boolean(mapRandoSettings.raceMode));
        json_object_object_add(other_settings_obj, "speed_booster", json_object_new_string(speedBoosterSplit));

        main_obj = json_object_new_object();
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
        json_object_object_add(main_obj, "other_settings", other_settings_obj);
    }
    //TRACE("Resulting JSON:\n%s\n", json_object_to_json_string_ext(main_obj, JSON_C_TO_STRING_PRETTY));

    // Send the first request
    const char *json_str = json_object_to_json_string_ext(main_obj, JSON_C_TO_STRING_PLAIN);
    char *response1 = send_request_1(mapRandoSettings.inputRomPath, mapRandoSettings.spoilerToken, json_str);
    
    // We can free main_obj now
    json_object_put(main_obj);

    if (!response1) {
        TRACE("Failed to send the first request.\n");
        return 1;
    }

    char *seedUrl = extract_seed_url(response1);
    free(response1);

    if (!seedUrl) {
        TRACE("Failed to extract seed URL.\n");
        return 1;
    }

    TRACE("Seed URL: %s%s\n", baseUrl, seedUrl);
    consoleUpdate(NULL);

    // Construct the customization URL
    char customize_url[512];
    snprintf(customize_url, sizeof(customize_url), "%s%scustomize", baseUrl, seedUrl);
    TRACE("%s", customize_url);

    char *seedPart = strstr(seedUrl, "/seed/");
    if (seedPart != NULL) {
        seedPart += strlen("/seed/");
        char *tempSeed = strdup(seedPart);
        size_t len = strlen(tempSeed);
        if (len > 0 && tempSeed[len - 1] == '/') {
            tempSeed[len - 1] = '\0';  // Remove the last character
        }
        char finalPath[256];
        snprintf(finalPath, sizeof(finalPath), "/map-rando-%s", tempSeed);
        int written = snprintf(
            outputPath,
            sizeof(outputPath),
            "%s%s.sfc",
            mapRandoSettings.outputRomPath,
            finalPath
        );
        free(tempSeed);

        if (written < 0 || written >= (int)sizeof(outputPath)) {
            TRACE("%s", "Output path too long\n");
            free(seedUrl);
            return 1;
        }

        TRACE("Combined Path: %s\n", outputPath);
    } else {
        TRACE("No valid seed part found.\n");
    }

    // Send the second request
    int res2 = send_request_2(customize_url, mapRandoSettings.inputRomPath, outputPath, mapRandoSettings);
    free(seedUrl);

    if (res2 != 0) {
        TRACE("Failed to send the second request.\n");
        return 1;
    }

    TRACE("Customized ROM saved as %s\n", outputPath);
    return 0;
}
