#pragma once
#include <inc/types.h>

class AddonImage {
public:
	AddonImage() : ModelHash(0), ResX(0), ResY(0), TextureID(0) {}
	AddonImage(int textureId, Hash hash, uint16_t resX, uint16_t resY) :
		ModelHash(hash),
		ResX(resX),
		ResY(resY),
		TextureID(textureId) { }

	Hash ModelHash;
	uint16_t ResX;
	uint16_t ResY;
	int TextureID;
};
