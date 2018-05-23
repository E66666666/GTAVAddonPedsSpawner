/*
THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
http://dev-c.com
(C) Alexander Blade 2015
*/

#pragma once

#define DISPLAY_VERSION "v0.0.0"

#include <string>
#include <vector>
#include <inc/types.h>

const std::string modDir  = "\\AddonPedsSpawner";

void ScriptMain();

void cacheAddons();

void update_menu();
void onMenuOpen();
void onMenuExit();
void spawnPed(Hash hash);
std::vector<std::string> resolvePedInfo(Hash ped);
std::string getModelName(Hash hash);
