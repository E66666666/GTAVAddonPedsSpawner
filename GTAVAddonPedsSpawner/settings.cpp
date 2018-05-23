#include "settings.h"
#include "simpleini/SimpleIni.h"

Settings::Settings() { }

Settings::~Settings() { }

void Settings::SetFiles(const std::string &general) {
	settingsGeneralFile = general;
}

void Settings::ReadSettings() {

	CSimpleIniA settingsGeneral;
	settingsGeneral.SetUnicode();
	settingsGeneral.LoadFile(settingsGeneralFile.c_str());
	
	SpawnByName = settingsGeneral.GetBoolValue("OPTIONS", "SpawnByName", false);
	SearchMenu = settingsGeneral.GetBoolValue("OPTIONS", "SearchMenu", false);
	SearchCategory = settingsGeneral.GetLongValue("OPTIONS", "SearchCategory", 0);
}


void Settings::SaveSettings() {
	CSimpleIniA settings;
	settings.SetUnicode();
	settings.LoadFile(settingsGeneralFile.c_str());

	settings.SetBoolValue("OPTIONS", "SpawnByName", SpawnByName);
	settings.SetBoolValue("OPTIONS", "SearchMenu", SearchMenu);
	settings.SetLongValue("OPTIONS", "SearchCategory", SearchCategory);

	settings.SaveFile(settingsGeneralFile.c_str());
}
