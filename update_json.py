import requests
from pathlib import Path

generateURL = "https://dev.maprando.com/generate"
seedURL = "https://dev.maprando.com/seed/ryKPRMrrn/"
jsonArrays = ["skillPresetsArr", "itemPresetsArr", "qolPresetsArr", "fullPresetsArr"]
parsedArrays = {}
settingsFileName = Path("source/settings.c")
maprandoFileName = Path("source/map_rando.c")
sprites = ""
presets = []
otherSettings = "{\"wall_jump\":\"Vanilla\",\"area_assignment\":{\"preset\":\"Standard\",\"base_order\":\"Size\",\"ship_in_crateria\":true,\"mother_brain_in_tourian\":true},\"door_locks_size\":\"Large\",\"map_station_reveal\":\"Full\",\"energy_free_shinesparks\":false,\"ultra_low_qol\":false,\"disable_spikesuit\":false,\"disable_bluesuit\":false,\"enable_major_glitches\":false,\"speed_booster\":\"Vanilla\",\"race_mode\":false,\"savestate\":\"No\",\"random_seed\":null},\"debug\":false}"

response = requests.get(generateURL)
if response.status_code == 200:
    source = response.text.splitlines()

    for line in source:
        if "<a class=\"nav-link m-1\"" in line.strip():
            print("Version:" + line.split(">")[1].split("<")[0])

    print("Generating settings.c...")

    for line in source:
        for array in jsonArrays:
            searchString = f"let {array}"
            if searchString in line:
               parsedArrays[array] = line.replace(f"    let {array} =[", "")
               # Handle presets
               if "fullPresetsArr" in line:
                    for preset in line.split("\"name\":\""):
                        if "skill_assumption_settings" in preset:
                            presets.append(preset.split("\",\"skill_assumption_settings")[0])

    print("Adding presets...")
    with open(maprandoFileName,'r') as inputFile:
        origFile = inputFile.readlines()
        origFile[45] = "const char *presets[] = {\"None\",\"" + "\",\"".join(presets) + "\"};\n"

        with open(maprandoFileName,'w') as outputFile:
            outputFile.writelines(origFile)

    for array in parsedArrays:
        json = parsedArrays[array]

        json = json[:-2]
        json = json.replace("\"", "\\\"")
        json = json.replace(",{\\\"preset\\\"", "\",\n\"{\\\"preset\\\"")
        json = json.replace(",{\\\"version\\\"", "\",\n\"{\\\"version\\\"")
        json = "const char *" + array + "[] = {\n\"" + json
        parsedArrays[array] = json

    outputFile = open(settingsFileName, "w")
    outputFile.write("#include \"settings.h\"\n\n// Settings Json for generating the randos\n\n")
    for array in parsedArrays:
        outputFile.write(parsedArrays[array] + "\"};" + "\n\n")
    outputFile.write("const char *otherSettings = \"" + otherSettings.replace("\"", "\\\"") + "\";")
    outputFile.close()
    print("settings.c done.")

print("Getting sprites...")
response = requests.get(seedURL)
if response.status_code == 200:
    source = response.text.splitlines()

    for line in source:
        if "data-name=" in line:
            for term in line.split(" "):
                if "data-name=" in term:
                    sprites = sprites + term.replace("data-name=", "") + ","

    print("Modifying map_rando.c...")
    with open(maprandoFileName,'r') as inputFile:
        origFile = inputFile.readlines()
        origFile[41] = "const char *suits[] = {" + sprites[:-1] + "};\n"

        with open(maprandoFileName,'w') as outputFile:
            outputFile.writelines(origFile)

    print("Getting tile themes...")
    tilesSource = ""
    tiles = ""
    j = 0
    i = 0
    for line in source:
        if "id=\"tileTheme\" name=\"tile_theme\" class=\"form-select\"" in line:
            j = i
            break
        i += 1
    while("</select>" not in source[j]):
        tilesSource = tilesSource + source[j]
        j+=1
    for term in tilesSource.split(" "):
        if "value=" in term:
            tiles = tiles + term.split(">")[0].replace("value=", "") + ","
    
    print("Modifying map_rando.c again...")
    with open(maprandoFileName,'r') as inputFile:
        origFile = inputFile.readlines()
        origFile[44] = "const char *tileTheme[] = {" + tiles[:-1] + "};\n"

        with open(maprandoFileName,'w') as outputFile:
            outputFile.writelines(origFile)

    print("Getting room palettes...")
    pallettesSource = ""
    pallettes = ""
    j = 0
    i = 0
    for line in source:
        if "<select id=\"roomPalettes\" name=\"room_palettes\" class=\"form-select\">" in line:
            j = i
            break
        i += 1
    while("</select>" not in source[j]):
        pallettesSource = pallettesSource + source[j]
        j+=1
    for term in pallettesSource.split(" "):
            if "value=" in term:
                pallettes = pallettes + term.split(">")[0].replace("value=", "") + ","

    print("Modifying map_rando.c once more...")
    with open(maprandoFileName,'r') as inputFile:
        origFile = inputFile.readlines()
        origFile[43] = "const char *roomPalettes[] = {" + pallettes[:-1] + "};\n"

        with open(maprandoFileName,'w') as outputFile:
            outputFile.writelines(origFile)

print("Done")