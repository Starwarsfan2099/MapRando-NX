#include "imgui.h"
#include "imgui_impl_glfw_switch.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <thread> // For std::thread

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define GLM_FORCE_PURE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#ifdef __SWITCH__
#include <switch.h>
#include "debug.h"
#include "map_rando.h"
#include "config.h"
#endif

#include <chrono>
#include <set>
#include <thread>

constexpr uint32_t WINDOW_WIDTH  = 1280;
constexpr uint32_t WINDOW_HEIGHT = 720;

#define DEFAULT_FPS 60

static void errorCallback(int errorCode, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", errorCode, description);
}

const char* qualityOfLifeLabels[] = {"Off", "Low", "Default", "High", "Max"};
const char* itemProgressionLabels[] = {"Normal", "Tricky", "Technical", "Challenge", "Desolate"};
const char* skillLevelLabels[] = {"Basic", "Medium", "Hard", "Very Hard", "Expert", "Extreme", "Insane"};

bool showPopup = false;
bool showSavePopup = false;
bool loadingInProgress = false;
bool loadingDone = false;
int loadingFrames = 0;
int success = 0;
bool internetConnection = true;

int main(int, char**)
{
    socketInitializeDefault();

    nifmInitialize(NifmServiceType_User);

    NifmInternetConnectionType connectionType;
    u32 wifiStrength;
    NifmInternetConnectionStatus connectionStatus;

    Result res = nifmGetInternetConnectionStatus(&connectionType, &wifiStrength, &connectionStatus);
    if (R_SUCCEEDED(res)) {
        if (connectionStatus == NifmInternetConnectionStatus_Connected) {
            TRACE("Internet is available!\n");
        } else {
            TRACE("Internet is NOT available.\n");
            internetConnection = false;
        }
    } else {
        TRACE("Failed to get internet connection status: 0x%08X\n", res);
        internetConnection = false;
    }

    #ifdef DEBUG
        initNxLink();
        TRACE("Starting init...");
    #endif
    // Init rng
    std::srand(std::time(nullptr));

    TRACE("Loading config file...");
    struct mapRando mapRandoSettings;
    loadSettingsFromFile(&mapRandoSettings, "/config/MapRando-NX/config.txt");
    
    // Init glfw
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit())
    {
        return false;
    }
    const char* glsl_version = "#version 430 core";

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "test", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return false;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwMakeContextCurrent(window);

    // Load OpenGL routines using glad
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.FontGlobalScale = 1.5f;
    ImGui::GetStyle().ScaleAllSizes(1.5f);
    
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    io.Fonts->AddFontDefault();
    {
        PlFontData standard, extended;
    static ImWchar extended_range[] = {0xe000, 0xe152};
    if (R_SUCCEEDED(plGetSharedFontByType(&standard,     PlSharedFontType_Standard)) &&
            R_SUCCEEDED(plGetSharedFontByType(&extended, PlSharedFontType_NintendoExt))) {
        std::uint8_t *px;
        int w, h, bpp;
        ImFontConfig font_cfg;

        font_cfg.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF(standard.address, standard.size, 20.0f, &font_cfg, io.Fonts->GetGlyphRangesDefault());
        font_cfg.MergeMode            = true;
        io.Fonts->AddFontFromMemoryTTF(extended.address, extended.size, 20.0f, &font_cfg, extended_range);

        io.Fonts->GetTexDataAsAlpha8(&px, &w, &h, &bpp);
        io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;
        io.Fonts->Build();
    }

    }
    TRACE("Done fonts");
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    int width, height;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    TRACE("Main Loop");
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Handle a few settings that change based on other settings
        if ((mapRandoSettings.tileTheme != 1) && (mapRandoSettings.tileTheme != 0)) {
            mapRandoSettings.roomTheming = 3;
        }

        if (mapRandoSettings.roomTheming == 0){
            mapRandoSettings.roomPalettes = 0;
        } else if (mapRandoSettings.roomTheming == 1) {
            mapRandoSettings.roomPalettes = 1;
            mapRandoSettings.tileTheme = 0;
        } else if (mapRandoSettings.roomTheming == 2) {
            mapRandoSettings.roomPalettes = 1;
            mapRandoSettings.tileTheme = 1;
        }

        glfwGetFramebufferSize(window, &width, &height);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(width, height));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("Map Rando Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        // Skill Assumptions Section
        ImGui::Text("Skill Assumptions");
        if (ImGui::BeginCombo("##SkillLevel", skillLevelLabels[mapRandoSettings.skillLevel])) {
            for (int i = 0; i < 7; i++) {
                if (ImGui::Selectable(skillLevelLabels[i], mapRandoSettings.skillLevel == i)) {
                    mapRandoSettings.skillLevel = i;
                }
            }
            ImGui::EndCombo();
        }

        // Item Progression Section
        ImGui::Text("Item Progression");
        if (ImGui::BeginCombo("##ItemProgression", itemProgressionLabels[mapRandoSettings.itemProgression])) {
            for (int i = 0; i < 5; i++) {
                if (ImGui::Selectable(itemProgressionLabels[i], mapRandoSettings.itemProgression == i)) {
                    mapRandoSettings.itemProgression = i;
                }
            }
            ImGui::EndCombo();
        }

        // Quality of Life Section
        ImGui::Text("Quality of Life Options");
        if (ImGui::BeginCombo("##QualityOfLife", qualityOfLifeLabels[mapRandoSettings.qualityOfLife])) {
            for (int i = 0; i < 5; i++) {
                if (ImGui::Selectable(qualityOfLifeLabels[i], mapRandoSettings.qualityOfLife == i)) {
                    mapRandoSettings.qualityOfLife = i;
                }
            }
            ImGui::EndCombo();
        }

        // Objectives Section
        ImGui::Text("Objectives");
        ImGui::RadioButton("None", &mapRandoSettings.objectives, 0); ImGui::SameLine();
        ImGui::RadioButton("Bosses", &mapRandoSettings.objectives, 1); ImGui::SameLine();
        ImGui::RadioButton("Minibosses", &mapRandoSettings.objectives, 2); ImGui::SameLine();
        ImGui::RadioButton("Metroids", &mapRandoSettings.objectives, 3); ImGui::SameLine();
        ImGui::RadioButton("Chozos", &mapRandoSettings.objectives, 4); ImGui::SameLine();
        ImGui::RadioButton("Pirates", &mapRandoSettings.objectives, 5); ImGui::SameLine();
        ImGui::RadioButton("Random##1", &mapRandoSettings.objectives, 6);

        // Map Layout
        ImGui::Text("Map Layout");
        ImGui::RadioButton("Vanilla##1", &mapRandoSettings.mapLayout, 0); ImGui::SameLine();
        ImGui::RadioButton("Small##1", &mapRandoSettings.mapLayout, 1); ImGui::SameLine();
        ImGui::RadioButton("Standard", &mapRandoSettings.mapLayout, 2); ImGui::SameLine();
        ImGui::RadioButton("Wild", &mapRandoSettings.mapLayout, 3);

        // Doors
        ImGui::Text("Doors");
        ImGui::RadioButton("Blue", &mapRandoSettings.doors, 0); ImGui::SameLine();
        ImGui::RadioButton("Ammo", &mapRandoSettings.doors, 1); ImGui::SameLine();
        ImGui::RadioButton("Beam", &mapRandoSettings.doors, 2);

        // Start Location
        ImGui::Text("Start Location");
        ImGui::RadioButton("Ship", &mapRandoSettings.startLocation, 0); ImGui::SameLine();
        ImGui::RadioButton("Random##2", &mapRandoSettings.startLocation, 1); ImGui::SameLine();
        ImGui::RadioButton("Escape", &mapRandoSettings.startLocation, 2);

        // Save the Animals
        ImGui::Text("Save the Animals");
        ImGui::RadioButton("No", &mapRandoSettings.saveAnimals, 0); ImGui::SameLine();
        ImGui::RadioButton("Yes", &mapRandoSettings.saveAnimals, 1); ImGui::SameLine();
        ImGui::RadioButton("Optional", &mapRandoSettings.saveAnimals, 2); ImGui::SameLine();
        ImGui::RadioButton("Random##3", &mapRandoSettings.saveAnimals, 3);

        // Other Options
        ImGui::Separator();

        ImGui::Text("Wall Jump:"); ImGui::SameLine();
        ImGui::RadioButton("Vanilla##2", &mapRandoSettings.wallJumpMode, 0); ImGui::SameLine();
        ImGui::RadioButton("Collectible", &mapRandoSettings.wallJumpMode, 1);

        ImGui::Text("E-Tank Energy Refill:"); ImGui::SameLine();
        ImGui::RadioButton("Disabled", &mapRandoSettings.eTankMode, 0); ImGui::SameLine();
        ImGui::RadioButton("Vanilla##3", &mapRandoSettings.eTankMode, 1); ImGui::SameLine();
        ImGui::RadioButton("Full", &mapRandoSettings.eTankMode, 2);

        ImGui::Text("Area Assignment:"); ImGui::SameLine();
        ImGui::RadioButton("Ordered", &mapRandoSettings.areaAssignment, 0); ImGui::SameLine();
        ImGui::RadioButton("Standard##2", &mapRandoSettings.areaAssignment, 1); ImGui::SameLine();
        ImGui::RadioButton("Random##4", &mapRandoSettings.areaAssignment, 2);

        ImGui::Text("Item Dots After Collection:"); ImGui::SameLine();
        ImGui::RadioButton("Fade", &mapRandoSettings.dotsFade, 0); ImGui::SameLine();
        ImGui::RadioButton("Disappear", &mapRandoSettings.dotsFade, 1);

        ImGui::Text("Area transition markers on map:"); ImGui::SameLine();
        ImGui::RadioButton("Arrows", &mapRandoSettings.mapArrows, 0); ImGui::SameLine();
        ImGui::RadioButton("Letters", &mapRandoSettings.mapArrows, 1);

        ImGui::Text("Door lock size on map:"); ImGui::SameLine();
        ImGui::RadioButton("Small##2", &mapRandoSettings.doorLock, 0); ImGui::SameLine();
        ImGui::RadioButton("Large", &mapRandoSettings.doorLock, 1);

        ImGui::Text("Map revealed from start:"); ImGui::SameLine();
        ImGui::RadioButton("No##2", &mapRandoSettings.mapRevealed, 0); ImGui::SameLine();
        ImGui::RadioButton("Partial", &mapRandoSettings.mapRevealed, 1); ImGui::SameLine();
        ImGui::RadioButton("Full##2", &mapRandoSettings.mapRevealed, 2);

        ImGui::Text("Map station activation reveal:"); ImGui::SameLine();
        ImGui::RadioButton("Partial##2", &mapRandoSettings.mapStation, 0); ImGui::SameLine();
        ImGui::RadioButton("Full##3", &mapRandoSettings.mapStation, 1);

        ImGui::Checkbox("Energy free shinespark", &mapRandoSettings.freeShinespark); ImGui::SameLine();
        ImGui::Checkbox("Ultra low quality of life", &mapRandoSettings.ultraQuality); ImGui::SameLine();
        ImGui::Checkbox("Race mode", &mapRandoSettings.raceMode);ImGui::SameLine();
        ImGui::Checkbox("Room names", &mapRandoSettings.roomNames);

        ImGui::Separator();

        // Suits
        ImGui::Text("Suit:");
        if (ImGui::BeginCombo("##Suit", suits[mapRandoSettings.suit])) {
            for (int i = 0; i < suits_size; i++) {
                if (ImGui::Selectable(suits[i], mapRandoSettings.suit == i)) {
                    mapRandoSettings.suit = i;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Text("Room theming:"); ImGui::SameLine();
        ImGui::RadioButton("Vanilla##4", &mapRandoSettings.roomTheming, 0); ImGui::SameLine();
        ImGui::RadioButton("Area Palettes", &mapRandoSettings.roomTheming, 1); ImGui::SameLine();
        ImGui::RadioButton("Area Tiling", &mapRandoSettings.roomTheming, 2);

        ImGui::Text("Door colors:"); ImGui::SameLine();
        ImGui::RadioButton("Vanilla##5", &mapRandoSettings.doorColors, 0); ImGui::SameLine();
        ImGui::RadioButton("Alternate", &mapRandoSettings.doorColors, 1);

        ImGui::Text("Music:"); ImGui::SameLine();
        ImGui::RadioButton("On", &mapRandoSettings.music, 0); ImGui::SameLine();
        ImGui::RadioButton("Off", &mapRandoSettings.music, 1);

        ImGui::Text("Screen shaking:"); ImGui::SameLine();
        ImGui::RadioButton("Vanilla##6", &mapRandoSettings.screenShaking, 0); ImGui::SameLine();
        ImGui::RadioButton("Reduced", &mapRandoSettings.screenShaking, 1); ImGui::SameLine();
        ImGui::RadioButton("Disabled##2", &mapRandoSettings.screenShaking, 2);

        ImGui::Text("Screen flashing:"); ImGui::SameLine();
        ImGui::RadioButton("Vanilla##7", &mapRandoSettings.screenFlashing, 0); ImGui::SameLine();
        ImGui::RadioButton("Reduced##2", &mapRandoSettings.screenFlashing, 1);

        ImGui::Text("Low energy beeping:"); ImGui::SameLine();
        ImGui::RadioButton("Vanilla##8", &mapRandoSettings.lowEnergyBeeping, 0); ImGui::SameLine();
        ImGui::RadioButton("Disabled##3", &mapRandoSettings.lowEnergyBeeping, 1);

        //Tile themes
        ImGui::Text("Tile theme:");
        if (ImGui::BeginCombo("##tileTheme", tileTheme[mapRandoSettings.tileTheme])) {
            for (int i = 0; i < tile_size; i++) {
                if (ImGui::Selectable(tileTheme[i], mapRandoSettings.tileTheme == i)) {
                    mapRandoSettings.tileTheme = i;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        // Show options from the config file
        ImGui::Text("Input file:"); ImGui::SameLine();
        ImGui::InputText(" ", mapRandoSettings.inputRomPath, IM_ARRAYSIZE(mapRandoSettings.inputRomPath));

        ImGui::Text("Output path:"); ImGui::SameLine();
        ImGui::InputText(" ##2", mapRandoSettings.outputRomPath, IM_ARRAYSIZE(mapRandoSettings.outputRomPath));

        ImGui::Text("Spoiler token:"); ImGui::SameLine();
        ImGui::InputText(" ##3", mapRandoSettings.spoilerToken, IM_ARRAYSIZE(mapRandoSettings.spoilerToken));
        
        // Generate Button
        ImGui::SetCursorPosX(width - 250);
        ImGui::SetCursorPosY(40);
        if (ImGui::Button("Generate Rando", ImVec2(200, 50))) {
            showPopup = true;
            loadingInProgress = true;
            loadingDone = false;
            loadingFrames = 0;
        }

        // Save settings button
        ImGui::SetCursorPosX(width - 235);
        ImGui::SetCursorPosY(100);
        if (ImGui::Button("Save settings", ImVec2(170, 40))) {
            success = saveSettingsToFile(&mapRandoSettings, "/config/MapRando-NX/config.txt");
            showSavePopup = true;
        }

        ImGui::End();

        // Saving popup
        if (showSavePopup){
            ImGui::OpenPopup("Save settings");
            ImVec2 screen_size = ImGui::GetIO().DisplaySize;
            ImVec2 window_size = ImVec2(400, 200);
            ImVec2 window_pos = ImVec2((screen_size.x - window_size.x) * 0.5f, 
                                    (screen_size.y - window_size.y) * 0.5f);

            // Set the window position and size
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(window_size);
            if (ImGui::BeginPopupModal("Save settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::SetWindowFontScale(1.5f);
                if (success == 0){
                    ImGui::Text("Complete!");
                } else {
                    ImGui::Text("Saving Failed.");
                }
                if (ImGui::Button("Close")) {
                    showSavePopup = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        // "Generating" popup
        if (showPopup) {
            ImGui::OpenPopup("Generating Map Rando");
            ImVec2 screen_size = ImGui::GetIO().DisplaySize;
            ImVec2 window_size = ImVec2(800, 200);
            ImVec2 window_pos = ImVec2((screen_size.x - window_size.x) * 0.5f, 
                                    (screen_size.y - window_size.y) * 0.5f);

            // Set the window position and size
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(window_size);
            if (ImGui::BeginPopupModal("Generating Map Rando", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::SetWindowFontScale(1.5f);
                if (loadingInProgress) {
                    if (internetConnection) {
                        ImGui::Text("Generating...");
                        loadingFrames++;
                        if (loadingFrames > 3) {
                            loadingInProgress = false;
                            loadingDone = true;
                        }
                    } else {
                        ImGui::Text("No internet connection.");
                    }
                } else if (loadingDone) {
                    success = generate_map_rando(mapRandoSettings);
                    loadingDone = false;
                } else {
                    if (success == 0){
                        ImGui::Text("Complete! Saved to:");
                        ImGui::Text(outputPath); 
                    } else {
                        ImGui::Text("Generating Failed.");
                    }
                    if (ImGui::Button("Close")) {
                        showPopup = false;
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndPopup();
            }
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus)
            break;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    socketExit();

    return 0;
}
