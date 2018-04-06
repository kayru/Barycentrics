#pragma once

#include <string>
#include <Rush/Platform.h>
#include <Rush/UtilTimer.h>
#include <Rush/GfxCommon.h>

template <typename T, size_t SIZE>
struct MovingAverage
{
	MovingAverage() { reset(); }
	void reset() { idx = 0; sum = 0; for(T& it : buf) it=0; }
	T get() const { return sum / SIZE; }
	void add(T v)
	{
		sum += v;
		sum -= buf[idx];
		buf[idx] = v;
		idx = (idx + 1) % SIZE;
	}
	size_t idx;
	T sum;
	T buf[SIZE];
};

template <typename T, size_t S>
struct TimingScope
{

	TimingScope(MovingAverage<T, S>& output)
		: m_output(output)
	{}

	~TimingScope()
	{
		m_output.add(m_timer.time());
	}

	MovingAverage<T, S>& m_output;
	Timer m_timer;
};

inline u64 hashFnv1a64(const void* message, size_t length, u64 state = 0xcbf29ce484222325)
{
	const u8* bytes = (const u8*)message;
	for (size_t i = 0; i < length; ++i)
	{
		state ^= bytes[i];
		state *= 0x100000001b3;
	}
	return state;
}

std::string directoryFromFilename(const std::string& filename);
GfxShaderSource shaderFromFile(const char* filename, const char* shaderDirectory = Platform_GetExecutableDirectory());
GfxTexture textureFromFile(const char* filename);
GfxTexture generateMipsRGBA8(u8* pixels, int w, int h);
