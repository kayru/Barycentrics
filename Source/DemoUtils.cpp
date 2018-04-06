#include "DemoUtils.h"

#include <Rush/GfxCommon.h>
#include <Rush/GfxDevice.h>
#include <Rush/UtilFile.h>

#pragma warning(push)
#pragma warning(disable: 4996)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>
#pragma warning(pop)

#include <vector>
#include <memory>

std::string directoryFromFilename(const std::string& filename)
{
	size_t pos = filename.find_last_of("/\\");
	if (pos != std::string::npos)
	{
		return filename.substr(0, pos + 1);
	}
	else
	{
		return std::string();
	}
}

GfxShaderSource shaderFromFile(const char* filename, const char* shaderDirectory)
{
	std::string fullFilename = std::string(shaderDirectory) + "/" + std::string(filename);
	Log::message("Loading shader '%s'", filename);

	GfxShaderSource source;
	source.type = GfxShaderSourceType_SPV;

	FileIn f(fullFilename.c_str());
	if (f.valid())
	{
		u32 fileSize = f.length();
		source.resize(fileSize);
		f.read(&source[0], fileSize);
	}

	if (source.empty())
	{
		Log::error("Failed to load shader '%s'", filename);
	}

	return source;
};

GfxTexture textureFromFile(const char* filename)
{
	int w, h, comp;
	stbi_set_flip_vertically_on_load(true);
	u8* pixels = stbi_load(filename, &w, &h, &comp, 4);

	GfxTexture result;

	if (pixels)
	{
		result = generateMipsRGBA8(pixels, w, h);
		stbi_image_free(pixels);
	}
	else
	{
		Log::warning("Failed to load texture '%s'", filename);
	}

	return result;
}

GfxTexture generateMipsRGBA8(u8* pixels, int w, int h)
{
	GfxTexture result;

	std::vector<std::unique_ptr<u8>> mips;
	mips.reserve(16);

	std::vector<GfxTextureData> textureData;
	textureData.reserve(16);
	textureData.push_back(GfxTextureData(pixels));

	u32 mipWidth = w;
	u32 mipHeight = h;

	while (mipWidth != 1 && mipHeight != 1)
	{
		u32 nextMipWidth = max<u32>(1, mipWidth / 2);
		u32 nextMipHeight = max<u32>(1, mipHeight / 2);

		u8* nextMip = new u8[nextMipWidth * nextMipHeight * 4];
		mips.push_back(std::unique_ptr<u8>(nextMip));

		const u32 mipPitch = mipWidth * 4;
		const u32 nextMipPitch = nextMipWidth * 4;
		int resizeResult = stbir_resize_uint8(
			(const u8*)textureData.back().pixels, mipWidth, mipHeight, mipPitch,
			nextMip, nextMipWidth, nextMipHeight, nextMipPitch, 4);
		RUSH_ASSERT(resizeResult);

		textureData.push_back(GfxTextureData(nextMip, (u32)textureData.size()));

		mipWidth = nextMipWidth;
		mipHeight = nextMipHeight;
	}

	GfxTextureDesc desc = GfxTextureDesc::make2D(w, h);
	desc.mips = (u32)textureData.size();

	result = Gfx_CreateTexture(desc, textureData.data(), (u32)textureData.size());	
	return result;
}

