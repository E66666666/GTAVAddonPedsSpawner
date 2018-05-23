#pragma once

#include <cstdint>
#include <vector>
#include "inc/natives.h"

#define STATIC_ASSERT_SIZE(Type, Size) static_assert(sizeof(Type) == Size, "invalid " #Type " size")

typedef uint8_t eModelType;

// Various snippets from FiveM source and Unknown Modder
#pragma pack(push, 1)
namespace rage {
	class grcTexture {
	public:
		void* VTable; // 0x0000
		char _0x0008[0x20]; // 0x0008
		char* name; // 0x0028
		char _0x0030[0x14]; // 0x0030
		uint32_t unk_0x0044; // 0x0044
		char _0x0048[0x8]; // 0x0048
		uint16_t resolutionX; // 0x0050
		uint16_t resolutionY; // 0x0052
		char _0x0054[0xC]; // 0x0054
		grcTexture* previous; // 0x0060
		grcTexture* next; // 0x0068
		char _0x0070[0x20]; // 0x0070
	};

	class pgDictionary {
	public:
		char _0x0000[0x30]; // 0x0000
		grcTexture** textures; // 0x0030
		uint16_t textureCount; // 0x0038
	};

    class fwArchetype {
    public:
        virtual ~fwArchetype() = default;

        char _0x0008[0x10]; // 0x0000
        Hash m_hash; // 0x0018
        char _0x001C[0x10]; // 0x001C
        float m_radius; // 0x002C
        float m_aabbMin[4]; // 0x0030
        float m_aabbMax[4]; // 0x0040
        uint32_t m_flags; // 0x0050
        char _0x0054[0x12]; // 0x0054
        uint16_t m_index; // 0x0066
    };

    class fwEntity
    {
    public:
        virtual ~fwEntity() = 0;

        virtual bool IsOfType(uint32_t hash) = 0;

        template<typename T>
        bool IsOfType()
        {
            return reinterpret_cast<T*>(this->IsOfType(HashString(boost::typeindex::type_id<T>().pretty_name().substr(6).c_str())));
        }
    };

    class fwArchetypeDef
    {
    public:
        virtual ~fwArchetypeDef();

        virtual int64_t GetTypeIdentifier();

        float lodDist;
        uint32_t flags; // 0x10000 = alphaclip
        uint32_t specialAttribute; // lower 5 bits == 31 -> use alpha clip, get masked to 31 in InitializeFromArchetypeDef
        uint32_t pad;
        void* pad2;
        float bbMin[4];
        float bbMax[4];
        float bsCentre[4];
        float bsRadius;
        float hdTextureDist;
        uint32_t name;
        uint32_t textureDictionary;
        uint32_t clipDictionary;
        uint32_t drawableDictionary;
        uint32_t physicsDictionary;
        uint32_t assetType;
        uint32_t assetName;
        uint32_t pad5[7];

    public:
        fwArchetypeDef()
        {
            flags = 0x10000; // was 0x2000
            lodDist = 299.0f;
            hdTextureDist = 375.0f;

            drawableDictionary = 0;
            assetType = 3;
            assetName = 0x12345678;

            specialAttribute = 31;

            pad = 0;
            pad2 = 0;
            clipDictionary = 0;
            physicsDictionary = 0;
            memset(pad5, 0, sizeof(physicsDictionary));
        }
    };

}

class CBaseModelInfo : public rage::fwArchetype
{
public:
    virtual ~CBaseModelInfo() {}
    virtual void Initialize() {}
    virtual void InitializeFromArchetypeDef(uint32_t, rage::fwArchetypeDef*, bool) {}
    virtual rage::fwEntity* CreateEntity() { return nullptr; }
    // and lots of other functions...

public:
    eModelType GetModelType() const
    {
        return m_modelType & 0x1F;
    }

protected:
    char _0x0068[0x35];	// 0x0068
    eModelType m_modelType;	// 0x009D (& 0x1F)
    char _0x009E[0x2];	// 0x009E
    uint32_t m_unkFlag;	// 0x00A0
    char _0x00A4[0x4];	// 0x00A4
    void* m_0x00A8;		// 0x00A8
};

STATIC_ASSERT_SIZE(CBaseModelInfo, 0xB0);

class CPedModelInfo : public CBaseModelInfo {
public:

};

struct ScriptHeader {
	char padding1[16];					//0x0
	unsigned char** codeBlocksOffset;	//0x10
	char padding2[4];					//0x18
	int codeLength;						//0x1C
	char padding3[4];					//0x20
	int localCount;						//0x24
	char padding4[4];					//0x28
	int nativeCount;					//0x2C
	__int64* localOffset;				//0x30
	char padding5[8];					//0x38
	__int64* nativeOffset;				//0x40
	char padding6[16];					//0x48
	int nameHash;						//0x58
	char padding7[4];					//0x5C
	char* name;							//0x60
	char** stringsOffset;				//0x68
	int stringSize;						//0x70
	char padding8[12];					//0x74
										//END_OF_HEADER

	bool IsValid() const { return codeLength > 0; }
	int CodePageCount() const { return (codeLength + 0x3FFF) >> 14; }
	int GetCodePageSize(int page) const {
		return (page < 0 || page >= CodePageCount() ? 0 : (page == CodePageCount() - 1) ? codeLength & 0x3FFF : 0x4000);
	}
	unsigned char* GetCodePageAddress(int page) const { return codeBlocksOffset[page]; }
	unsigned char* GetCodePositionAddress(int codePosition) const {
		return codePosition < 0 || codePosition >= codeLength ? NULL : &codeBlocksOffset[codePosition >> 14][codePosition & 0x3FFF];
	}
	char* GetString(int stringPosition)const {
		return stringPosition < 0 || stringPosition >= stringSize ? NULL : &stringsOffset[stringPosition >> 14][stringPosition & 0x3FFF];
	}

};

#pragma pack(pop)

struct ScriptTableItem {
	ScriptHeader* Header;
	char padding[4];
	int hash;

	inline bool IsLoaded() const {
		return Header != NULL;
	}
};

struct ScriptTable {
	ScriptTableItem* TablePtr;
	char padding[16];
	int count;
	ScriptTableItem* FindScript(int hash) {
		if (TablePtr == NULL) {
			return NULL;//table initialisation hasnt happened yet
		}
		for (int i = 0; i<count; i++) {
			if (TablePtr[i].hash == hash) {
				return &TablePtr[i];
			}
		}
		return NULL;
	}
};

struct GlobalTable {
	__int64** GlobalBasePtr;
	__int64* AddressOf(int index) const { return &GlobalBasePtr[index >> 18 & 0x3F][index & 0x3FFFF]; }
	bool IsInitialised()const { return *GlobalBasePtr != NULL; }
};

struct HashNode {
	int hash;
	UINT16 data;
	UINT16 padding;
	HashNode* next;
};

class MemoryAccess {
public:
	static uintptr_t FindPattern(const char *pattern, const char *mask, const char *startAddress, size_t size);
	static uintptr_t FindPattern(const char* pattern, const char* mask);
};

void setupHooks();
void removeHooks();
