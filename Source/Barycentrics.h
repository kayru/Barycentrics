#pragma once

#include <Rush/GfxBitmapFont.h>
#include <Rush/GfxDevice.h>
#include <Rush/GfxPrimitiveBatch.h>
#include <Rush/GfxRef.h>
#include <Rush/MathTypes.h>
#include <Rush/Platform.h>
#include <Rush/UtilCamera.h>
#include <Rush/UtilCameraManipulator.h>
#include <Rush/UtilTimer.h>
#include <Rush/Window.h>

#include "BaseApplication.h"
#include "DemoUtils.h"

#include <stdio.h>
#include <memory>
#include <string>
#include <unordered_map>

class BarycentricsApp : public BaseApplication
{
public:

	BarycentricsApp();
	~BarycentricsApp();

	void update() override;

private:

	void render();

	bool loadModel(const char* filename);

	Camera m_camera;
	Camera m_interpolatedCamera;

	CameraManipulator* m_cameraMan;

	GfxTechnique m_techniqueNonIndexed;
	GfxTechnique m_techniqueGeometryShader;
	GfxTechnique m_techniqueIndexed;
	GfxTechnique m_techniqueManual;
	GfxTechnique m_techniquePassthroughGS;
	GfxTechnique m_techniqueNativeAMD;

	GfxBuffer m_vertexBuffer;
	GfxBuffer m_indexBuffer;
	GfxBuffer m_constantBuffer;

	u32 m_indexCount = 0;
	u32 m_vertexCount = 0;

	struct Constants
	{
		Mat4 matView = Mat4::identity();
		Mat4 matProj = Mat4::identity();
		Mat4 matViewProj = Mat4::identity();
		Mat4 matWorld = Mat4::identity();
		Vec4 cameraPos = Vec4(0.0f);
	};

	Mat4 m_worldTransform = Mat4::identity();

	Box3 m_boundingBox;

	struct Vertex // TODO: make a packed version of this for GPU
	{
		Vec3 position;
		Vec3 normal;
		Vec2 texcoord;
	};

	std::string m_statusString;
	bool m_valid = false;

	struct MaterialConstants
	{
		Vec4 baseColor;
	};

	struct Material
	{
		GfxTextureRef albedoTexture;
		GfxBufferRef constantBuffer;
	};

	Material m_defaultMaterial;

	WindowEventListener m_windowEvents;

	float m_cameraScale = 1.0f;

	Timer m_timer;


	enum Timestamp
	{
		Timestamp_World,
		Timestamp_UI,
	};

	struct Stats
	{
		MovingAverage<double, 60> gpuTotal;
		MovingAverage<double, 60> gpuWorld;
		MovingAverage<double, 60> gpuUI;
		MovingAverage<double, 60> cpuTotal;
		MovingAverage<double, 60> cpuWorld;
		MovingAverage<double, 60> cpuUI;
	} m_stats;

	enum class Mode
	{
		Indexed,
		NonIndexed,
		GeometryShader,
		Manual,
		PassthroughGS,
		NativeAMD,
	} m_mode = Mode::NonIndexed;

	const char* toString(Mode m)
	{
		switch (m)
		{
		default: return "Unknown";
		case Mode::Indexed: return "Indexed";
		case Mode::NonIndexed: return "NonIndexed";
		case Mode::GeometryShader: return "GeometryShader";
		case Mode::Manual: return "Manual";
		case Mode::PassthroughGS: return "PassthroughGS";
		case Mode::NativeAMD: return "NativeAMD";
		}
	}
};

