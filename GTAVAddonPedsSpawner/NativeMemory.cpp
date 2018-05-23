/*
 * Credits & Acknowledgements:
 * InitPedModelInfo:  Unknown Modder
 */

#include "NativeMemory.hpp"

#include <Windows.h>
#include <Psapi.h>

#include <array>
#include <vector>
#include <unordered_map>

#include <minhook/include/MinHook.h>

#include "Util/Logger.hpp"
#include "Util/Util.hpp"

typedef CPedModelInfo*(*InitPedModelInfo_t)(const char*, bool, unsigned int);

extern std::unordered_map<Hash, std::string> g_pedModels;

InitPedModelInfo_t InitPedModelInfo_orig;

CPedModelInfo* InitPedModelInfo_hook(const char* name, bool isNonDLCModel, unsigned int a3) {
    g_pedModels.insert({ joaat(name), name });
    return InitPedModelInfo_orig(name, isNonDLCModel, a3);
}

void setupHooks() {
    if (MH_Initialize() != MH_OK)
    {
        logger.Write(FATAL, "MH_Initialize failed");
        return;
    }

    auto addr = MemoryAccess::FindPattern("\x41\x8A\xD6\x48\x69\xFF\x18\x01\x00\x00", "xxxxxxxxxx");
    if (!addr) {
        logger.Write(ERROR, "Couldn't find InitPedModelInfo");
        return;
    }
    addr = addr + *(int*)(addr + 15) + 19;
    logger.Write(INFO, "Found InitPedModelInfo at 0x%p", addr);

    if (MH_CreateHook((LPVOID)addr, &InitPedModelInfo_hook,
        reinterpret_cast<LPVOID*>(&InitPedModelInfo_orig)) != MH_OK)
    {
        logger.Write(FATAL, "MH_CreateHook failed");
        return;
    }

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
    {
        logger.Write(FATAL, "MH_EnableHook failed");
        return;
    }
}

void removeHooks() {
    if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK)
    {
        logger.Write(FATAL, "MH_DisableHook failed");
        return;
    }
    if (MH_Uninitialize() != MH_OK)
    {
        logger.Write(FATAL, "MH_Uninitialize failed");
        return;
    }
}

uintptr_t MemoryAccess::FindPattern(const char *pattern, const char *mask, const char* startAddress, size_t size) {
	const char* address_end = startAddress + size;
	const auto mask_length = static_cast<size_t>(strlen(mask) - 1);

	for (size_t i = 0; startAddress < address_end; startAddress++) {
		if (*startAddress == pattern[i] || mask[i] == '?') {
			if (mask[i + 1] == '\0') {
				return reinterpret_cast<uintptr_t>(startAddress) - mask_length;
			}
			i++;
		}
		else {
			i = 0;
		}
	}
	return 0;
}

uintptr_t MemoryAccess::FindPattern(const char* pattern, const char* mask) {
	MODULEINFO modInfo = { };
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &modInfo, sizeof(MODULEINFO));

	return FindPattern(pattern, mask, reinterpret_cast<const char *>(modInfo.lpBaseOfDll), modInfo.SizeOfImage);
}

