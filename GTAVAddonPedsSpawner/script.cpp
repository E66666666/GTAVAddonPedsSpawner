#include "script.h"

#include <array>
#include <experimental/filesystem>
#include <iomanip>
#include <set>
#include <sstream>
#include <vector>
#include <chrono>

#include <inc/natives.h>
#include <menu.h>
#include <menucontrols.h>
#include <NativeMemory.hpp>

#include "Util/Logger.hpp"
#include "Util/Paths.h"
#include "Util/Util.hpp"

#include "settings.h"
#include "PedHashes.h"
#include "ExtraTypes.h"
#include <fstream>

namespace fs = std::experimental::filesystem;

NativeMenu::Menu menu;
NativeMenu::MenuControls controls;
Settings settings;

std::string settingsGeneralFile;
std::string settingsMenuFile;

Player player;
Ped playerPed;

int prevNotification;

AddonImage noImage;

std::vector<Hash> g_missingImages;

std::vector<Hash> g_gamePeds;
std::vector<Hash> g_addonPeds;

std::vector<AddonImage> g_addonImages;
std::vector<Hash> g_addonImageHashes;

std::unordered_map<Hash, std::string> g_pedModels;

/*
 * Resolving images only when we need it. Should take just 1 tick after an option is selected.
 * Only runs once per image.
 */
void resolveImage(Hash selected) {
	std::string imgPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\img";
	for (auto &file : fs::directory_iterator(imgPath)) {
		Hash hash = joaat(fs::path(file).stem().string());
		if (hash != selected) continue;

		std::string fileName = fs::path(file).string();
		unsigned width;
		unsigned height;
		if (!GetIMGDimensions(fileName, &width, &height)) {
			width = 480;
			height = 270;
		}
		int handle = createTexture(fileName.c_str());
		g_addonImages.push_back(AddonImage(handle, hash, width, height));
		return;
	}
	g_missingImages.push_back(selected);
}

std::string getModelName(Hash hash) {
    auto modelIt = g_pedModels.find(hash);
    if (modelIt != g_pedModels.end()) return modelIt->second;
    return "NOTFOUND";
}

/*
 * Initialize add-ons and used classes. This is ran first so
 * the log outputs a thing.
 */
void cacheAddons() {
	if (!g_addonPeds.empty())
		return;

	std::vector<Hash> allPeds;
    for (auto hash : g_pedModels) {
        allPeds.push_back(hash.first);
    }

	int hashLength = 12;
	int nameLength = 20;
	std::stringstream thingy;
	thingy << std::left << std::setw(hashLength) << std::setfill(' ') << "Hash";
	thingy << std::left << std::setw(nameLength) << std::setfill(' ') << "Model name";
	logger.Write(INFO, thingy.str());

	for (auto hash : allPeds) {
		if (std::find(g_gamePeds.begin(), g_gamePeds.end(), hash) == g_gamePeds.end()) {            
			std::stringstream hashAsHex;
			std::stringstream logStream;
			hashAsHex << "0x" << std::setfill('0') << std::setw(8) << std::uppercase << std::hex << hash;
			logStream << std::left << std::setw(hashLength) << std::setfill(' ') << hashAsHex.str();
			logStream << std::left << std::setw(nameLength) << std::setfill(' ') << getModelName(hash);
			logger.Write(INFO, logStream.str());

			g_addonPeds.push_back(hash);
		}
	}
}

bool findStringInNames(std::string search, Hash hash) {
    return findSubstring(getModelName(hash), search) != -1;
}

void spawnPed(Hash hash) {
	
}

std::string getImageExtra(Hash addonVehicle) {
    std::string extra;
    AddonImage addonImage;
    if (isHashInImgVector(addonVehicle, g_addonImages, &addonImage)) {
        extra = menu.ImagePrefix + std::to_string(addonImage.TextureID) +
            "W" + std::to_string(addonImage.ResX) +
            "H" + std::to_string(addonImage.ResY);
    }
    else if (find(g_addonImageHashes.begin(), g_addonImageHashes.end(), addonVehicle) != g_addonImageHashes.end()) {
        resolveImage(addonVehicle);
    }
    else {
        if (find(g_missingImages.begin(), g_missingImages.end(), addonVehicle) == g_missingImages.end()) {
            resolveImage(addonVehicle);
        }
    }
    if (extra.empty()) {
        extra = menu.ImagePrefix + std::to_string(noImage.TextureID) +
            "W" + std::to_string(noImage.ResX) +
            "H" + std::to_string(noImage.ResY);
    }
    return extra;
}

/*
 * Used by the menu so it gets only the info of the current addon vehicle option,
 * instead of everything. 
 */
std::vector<std::string> resolvePedInfo(Hash ped) {
	std::vector<std::string> extras;
	extras.push_back(getImageExtra(ped));
	extras.push_back("Model: \t" + to_lower(getModelName(ped)));
	return extras;
}

void update_game() {
	player = PLAYER::PLAYER_ID();
	playerPed = PLAYER::PLAYER_PED_ID();

	if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) {
		menu.CloseMenu();
		return;
	}

	update_menu();
}

/*
 * Since the scripts can be reloaded for dev stuff it'd be nice to just cache
 * the results. InitPedModelInfo _should_ only be called on game init, so
 * subsequent reloads make for an empty g_vehicleHashes. Cache is updated each
 * full game launch since g_pedModels isn't empty.
 */
void checkCache(std::string cacheFile) {
    if (g_pedModels.size() != 0) {
        std::ofstream outfile;
        outfile.open(cacheFile, std::ofstream::out | std::ofstream::trunc);
        for (auto hash : g_pedModels) {
            std::string line = std::to_string(hash.first) + " " + hash.second + "\n";
            outfile << line;
        }
    }
    else {
        std::ifstream infile(cacheFile);
        if (infile.is_open()) {
            Hash hash;
            std::string name;
            while (infile >> hash >> name) {
                g_pedModels.insert({ hash, name });
            }
        }
    }
}

void main() {
    // logger.SetMinLevel(DEBUG);
	logger.Write(INFO, "Script started");

	settingsGeneralFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\settings_general.ini";
	settingsMenuFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\settings_menu.ini";
	settings.SetFiles(settingsGeneralFile);
	settings.ReadSettings();

	menu.RegisterOnMain(std::bind(onMenuOpen));
	menu.RegisterOnExit(std::bind(onMenuExit));
	menu.SetFiles(settingsMenuFile);
	menu.ReadSettings();

	logger.Write(INFO, "Settings read");

    std::string cacheFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\hashes.cache";
    checkCache(cacheFile);

	cacheAddons();
//	cacheDLCs();

	Hash hash = joaat("noimage");
	std::string fileName = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\img\\noimage.png";
	if (FileExists(fileName)) {
		unsigned width;
		unsigned height;
		if (!GetIMGDimensions(fileName, &width, &height)) {
			width = 800;
			height = 450;
			logger.Write(WARN, "Failed to get image proportions for noimage.png, using default values");
		}
		int handle = createTexture(fileName.c_str());
		noImage = AddonImage(handle, hash, width, height);
	}
	else {
		unsigned width = 480;
		unsigned height = 270;
		noImage = AddonImage(-1, hash, width, height);
		logger.Write(ERROR, "Missing img/noimage.png!");
	}
	
	logger.Write(INFO, "Initialization finished");

	while (true) {
		update_game();
		WAIT(0);
	}
}

void clearGlobals() {
	g_missingImages.clear();
	g_addonPeds.clear();
	g_addonImages.clear();
    g_addonImageHashes.clear();
	g_gamePeds.clear();
}

void ScriptMain() {
	clearGlobals();
	main();
}
