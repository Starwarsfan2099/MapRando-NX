import requests
from pathlib import Path

generateURL = "https://dev.maprando.com/generate"
seedURL = "https://dev.maprando.com/seed/JfWx9J7Kt/"
jsonArrays = ["skillPresetsArr", "itemPresetsArr", "qolPresetsArr"]
parsedArrays = {}
settingsFileName = Path("source/settings.c")
maprandoFileName = Path("source/map_rando.c")
sprites = ""
otherSettings = "{\"wall_jump\":\"Vanilla\",\"area_assignment\":\"Standard\",\"item_dot_change\":\"Fade\",\"transition_letters\":true,\"door_locks_size\":\"Large\",\"maps_revealed\":\"No\",\"map_station_reveal\":\"Full\",\"energy_free_shinesparks\":false,\"ultra_low_qol\":false,\"race_mode\":false,\"random_seed\":null},\"debug\":false}"

print("Generating settings.c...")
response = requests.get(generateURL)
if response.status_code == 200:
    source = response.text.splitlines()

    for line in source:
        for array in jsonArrays:
            searchString = f"let {array}"
            if searchString in line:
               parsedArrays[array] = line.replace(f"    let {array} =[", "")

    for array in parsedArrays:
        json = parsedArrays[array]

        json = json[:-2]
        json = json.replace("\"", "\\\"")
        json = json.replace(",{\\\"preset\\\"", "\",\n\"{\\\"preset\\\"")
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
        origFile[39] = "const char *suits[] = {" + sprites[:-1] + "};\n"

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
    while (j < i + 10):
        tilesSource = tilesSource + source[j]
        j+=1
    for term in tilesSource.split(" "):
        if "value=" in term:
            tiles = tiles + term.split(">")[0].replace("value=", "") + ","
    
    print("Modifying map_rando.c again...")
    with open(maprandoFileName,'r') as inputFile:
        origFile = inputFile.readlines()
        origFile[42] = "const char *tileTheme[] = {" + tiles[:-1] + "};\n"

        with open(maprandoFileName,'w') as outputFile:
            outputFile.writelines(origFile)

print("Done")