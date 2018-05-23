#pragma once
#include <vector>
#include <simpleini/SimpleIni.h>

namespace NativeMenu {
class Menu;
class MenuControls;
}

class Settings
{
public:
	Settings();
	~Settings();
	void ReadSettings();
	void SaveSettings();
	void SetFiles(const std::string &general);

	bool SpawnByName = false;
	bool SearchMenu = false;
	int SearchCategory = 0;

private:
	std::string settingsGeneralFile;
	std::string settingsMenuFile;
};
