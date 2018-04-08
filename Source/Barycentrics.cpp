#include "Barycentrics.h"

#include <Rush/UtilFile.h>
#include <Rush/UtilLog.h>
#include <Rush/MathTypes.h>

#pragma warning(push)
#pragma warning(disable: 4996)
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#pragma warning(pop)

#include <meshoptimizer.h>

#include <algorithm>
#include <cmath>

AppConfig g_appConfig;

int main(int argc, char** argv)
{
	AppConfig& cfg = g_appConfig;

	cfg.name = "Barycentrics (" RUSH_RENDER_API_NAME ")";

	cfg.width = 1280;
	cfg.height = 720;
	cfg.argc = argc;
	cfg.argv = argv;
	cfg.resizable = true;

#ifndef NDEBUG
	cfg.debug = true;
	Log::breakOnError = true;
#endif

	return Platform_Main<BarycentricsApp>(cfg);
}

BarycentricsApp::BarycentricsApp()
	: BaseApplication()
	, m_boundingBox(Vec3(0.0f), Vec3(0.0f))
{
	Gfx_SetPresentInterval(1);

	m_windowEvents.setOwner(m_window);

	GfxShaderBindings bindings;
	bindings.addConstantBuffer("constantBuffer0", 0); // scene consants
	bindings.addConstantBuffer("constantBuffer1", 1); // material constants
	bindings.addCombinedSampler("sampler0", 2); // albedo texture sampler
	bindings.addStorageBuffer("vertexBuffer", 3);
	bindings.addStorageBuffer("indexBuffer", 4);

	GfxVertexFormatDesc vfDefaultDesc; // TODO: use de-interleaved vertex streams and packed vertices
	vfDefaultDesc.add(0, GfxVertexFormatDesc::DataType::Float3, GfxVertexFormatDesc::Semantic::Position, 0);
	vfDefaultDesc.add(0, GfxVertexFormatDesc::DataType::Float2, GfxVertexFormatDesc::Semantic::Texcoord, 0);

	GfxVertexFormatDesc vfEmptyDesc;

	GfxVertexShaderRef vsIndexed;
	vsIndexed.takeover(Gfx_CreateVertexShader(shaderFromFile("Shaders/ModelIndexed.vert.spv")));

	struct SpecializationData { u32 useTexture; };
	GfxSpecializationConstant specializationConstantLayout;
	specializationConstantLayout.id = 0;
	specializationConstantLayout.offset = 0;
	specializationConstantLayout.size = sizeof(SpecializationData);

	enum { specializationCount = 2 };
	SpecializationData specializationData[specializationCount] = { 0, 1 }; // non-textured and textured variants

	auto setupSpecialization = [&](GfxTechniqueDesc& techniqueDesc, u32 variantIndex)
	{
		techniqueDesc.specializationConstants = &specializationConstantLayout;
		techniqueDesc.specializationConstantCount = 1;
		techniqueDesc.specializationData = &specializationData[variantIndex];
		techniqueDesc.specializationDataSize = sizeof(SpecializationData);
	};

	{
		GfxVertexShaderRef vs;
		vs.takeover(Gfx_CreateVertexShader(shaderFromFile("Shaders/Model.vert.spv")));

		GfxPixelShaderRef ps;
		ps.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/Model.frag.spv")));

		GfxVertexFormatRef vf;
		vf.takeover(Gfx_CreateVertexFormat(vfEmptyDesc));

		GfxTechniqueDesc techniqueDesc(ps.get(), vs.get(), vf.get(), &bindings);

		for (u32 i=0; i<specializationCount; ++i)
		{
			setupSpecialization(techniqueDesc, i);
			m_techniqueNonIndexed[i].takeover(Gfx_CreateTechnique(techniqueDesc));
		}
	}

	{
		GfxPixelShaderRef ps;
		ps.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/Model.frag.spv")));

		GfxGeometryShaderRef gs;
		gs.takeover(Gfx_CreateGeometryShader(shaderFromFile("Shaders/ModelBarycentrics.geom.spv")));

		GfxVertexFormatRef vf;
		vf.takeover(Gfx_CreateVertexFormat(vfDefaultDesc));

		GfxTechniqueDesc techniqueDesc(ps.get(), vsIndexed.get(), vf.get(), &bindings);
		techniqueDesc.gs = gs.get();

		for (u32 i = 0; i < specializationCount; ++i)
		{
			setupSpecialization(techniqueDesc, i);
			m_techniqueGeometryShader[i].takeover(Gfx_CreateTechnique(techniqueDesc));
		}
	}

	{
		GfxPixelShaderRef ps;
		ps.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/ModelIndexed.frag.spv")));

		GfxVertexFormatRef vf;
		vf.takeover(Gfx_CreateVertexFormat(vfDefaultDesc));

		GfxTechniqueDesc techniqueDesc(ps.get(), vsIndexed.get(), vf.get(), &bindings);

		for (u32 i = 0; i < specializationCount; ++i)
		{
			setupSpecialization(techniqueDesc, i);
			m_techniqueIndexed[i].takeover(Gfx_CreateTechnique(techniqueDesc));
		}
	}

	{
		GfxVertexShaderRef vs;
		vs.takeover(Gfx_CreateVertexShader(shaderFromFile("Shaders/ModelManual.vert.spv")));

		GfxPixelShaderRef ps;
		ps.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/ModelManual.frag.spv")));

		GfxVertexFormatRef vf;
		vf.takeover(Gfx_CreateVertexFormat(vfDefaultDesc));

		GfxTechniqueDesc techniqueDesc(ps.get(), vs.get(), vf.get(), &bindings);

		for (u32 i = 0; i < specializationCount; ++i)
		{
			setupSpecialization(techniqueDesc, i);
			m_techniqueManual[i].takeover(Gfx_CreateTechnique(techniqueDesc));
		}
	}

	if (Gfx_GetCapability().geometryShaderPassthroughNV)
	{
		GfxVertexShaderRef vs;
		vs.takeover(Gfx_CreateVertexShader(shaderFromFile("Shaders/ModelPassthrough.vert.spv")));

		GfxPixelShaderRef ps;
		ps.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/ModelPassthrough.frag.spv")));

		GfxPixelShaderRef psTextured;
		psTextured.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/ModelPassthroughTextured.frag.spv")));

		GfxGeometryShaderRef gs;
		gs.takeover(Gfx_CreateGeometryShader(shaderFromFile("Shaders/ModelPassthrough.geom.spv")));

		GfxVertexFormatRef vf;
		vf.takeover(Gfx_CreateVertexFormat(vfDefaultDesc));

		GfxTechniqueDesc techniqueDesc(ps.get(), vs.get(), vf.get(), &bindings);
		techniqueDesc.gs = gs.get();

		// Explicit precompiled textured variant is used due to gl_PrimitiveID overhead
		// being present even when specialization constant is 'false'.

		m_techniquePassthroughGS[0].takeover(Gfx_CreateTechnique(techniqueDesc));

		techniqueDesc.ps = psTextured.get();
		m_techniquePassthroughGS[1].takeover(Gfx_CreateTechnique(techniqueDesc));
	}

	if (Gfx_GetCapability().explicitVertexParameterAMD)
	{
		GfxVertexShaderRef vs;
		vs.takeover(Gfx_CreateVertexShader(shaderFromFile("Shaders/ModelNativeAMD.vert.spv")));

		GfxPixelShaderRef ps;
		ps.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/ModelNativeAMD.frag.spv")));

		GfxPixelShaderRef psTextured;
		psTextured.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/ModelNativeAMDTextured.frag.spv")));

		GfxVertexFormatRef vf;
		vf.takeover(Gfx_CreateVertexFormat(vfDefaultDesc));

		// Explicit precompiled textured variant is used due to gl_PrimitiveID overhead
		// being present even when specialization constant is 'false'.

		GfxTechniqueDesc techniqueDesc(ps.get(), vs.get(), vf.get(), &bindings);
		m_techniqueNativeAMD[0].takeover(Gfx_CreateTechnique(techniqueDesc));

		techniqueDesc.ps = psTextured.get();
		m_techniqueNativeAMD[1].takeover(Gfx_CreateTechnique(techniqueDesc));
	}

	GfxBufferDesc cbDescr(GfxBufferFlags::TransientConstant, GfxFormat_Unknown, 1, sizeof(Constants));
	m_constantBuffer = Gfx_CreateBuffer(cbDescr);

	{
		const GfxBufferDesc materialCbDesc(GfxBufferFlags::Constant, GfxFormat_Unknown, 1, sizeof(MaterialConstants));
		MaterialConstants constants;
		constants.baseColor = Vec4(1.0f);
		m_defaultMaterial.constantBuffer.takeover(Gfx_CreateBuffer(materialCbDesc, &constants));
		m_defaultMaterial.albedoTexture.retain(m_checkerboardTexture);
	}

	float aspect = m_window->getAspect();
	float fov = 1.0f;

	m_camera = Camera(aspect, fov, 0.25f, 10000.0f);

	if (g_appConfig.argc >= 2)
	{
		const char* modelFilename = g_appConfig.argv[1];
		m_statusString = std::string("Model: ") + modelFilename;
		m_valid = loadModel(modelFilename);

		Vec3 center = m_boundingBox.center();
		Vec3 dimensions = m_boundingBox.dimensions();
		float longestSide = dimensions.reduceMax();
		if (longestSide != 0)
		{
			float scale = 100.0f / longestSide;
			m_worldTransform = Mat4::scaleTranslate(scale, -center*scale);
		}

		m_boundingBox.m_min = m_worldTransform * m_boundingBox.m_min;
		m_boundingBox.m_max = m_worldTransform * m_boundingBox.m_max;

		m_camera.lookAt(Vec3(m_boundingBox.m_max) + Vec3(2.0f), m_boundingBox.center());
	}
	else
	{
		// Default tunnel test model
		m_valid = loadTunnelTestModel();

		Vec3 position = m_boundingBox.center();
		position.z = m_boundingBox.m_min.z;
		m_camera.lookAt(position, m_boundingBox.center());
	}

	m_interpolatedCamera = m_camera;

	m_cameraMan = new CameraManipulator();
}

BarycentricsApp::~BarycentricsApp()
{
	m_windowEvents.setOwner(nullptr);

	delete m_cameraMan;

	Gfx_Release(m_vertexBuffer);
	Gfx_Release(m_indexBuffer);
	Gfx_Release(m_constantBuffer);
}

void BarycentricsApp::update()
{
	TimingScope<double, 60> timingScope(m_stats.cpuTotal);

	m_stats.gpuWorld.add(Gfx_Stats().customTimer[Timestamp_World]);
	m_stats.gpuUI.add(Gfx_Stats().customTimer[Timestamp_UI]);
	m_stats.gpuTotal.add(Gfx_Stats().lastFrameGpuTime);

	Gfx_ResetStats();

	const float dt = (float)m_timer.time();
	m_timer.reset();

	for (const WindowEvent& e : m_windowEvents)
	{
		switch (e.type)
		{
		case WindowEventType_Scroll:
			if (e.scroll.y > 0)
			{
				m_cameraScale *= 1.25f;
			}
			else
			{
				m_cameraScale *= 0.9f;
			}
			Log::message("Camera scale: %f", m_cameraScale);
			break;
		case WindowEventType_KeyDown:
		{
			if (e.code == Key_0)
			{
				m_mode = Mode::Indexed;
			}
			else if (e.code == Key_1)
			{
				m_mode = Mode::NonIndexed;
			}
			else if (e.code == Key_2)
			{
				m_mode = Mode::GeometryShader;
			}
			else if (e.code == Key_3)
			{
				m_mode = Mode::Manual;
			}
			else if (e.code == Key_4 && m_techniquePassthroughGS[m_useTexture].valid())
			{
				m_mode = Mode::PassthroughGS;
			}
			else if (e.code == Key_5 && m_techniqueNativeAMD[m_useTexture].valid())
			{
				m_mode = Mode::NativeAMD;
			}
			else if (e.code == Key_T)
			{
				m_useTexture = !m_useTexture;
			}
			else if (e.code == Key_H)
			{
				m_showUI = !m_showUI;
			}
			break;
		}
		default:
			break;
		}
	}

	float clipNear = 0.25f * m_cameraScale;
	float clipFar = 10000.0f * m_cameraScale;
	m_camera.setClip(clipNear, clipFar);
	m_camera.setAspect(m_window->getAspect());
	m_cameraMan->setMoveSpeed(20.0f * m_cameraScale);

	m_cameraMan->update(&m_camera, dt, m_window->getKeyboardState(), m_window->getMouseState());

	m_interpolatedCamera.blendTo(m_camera, 0.1f, 0.125f);

	m_windowEvents.clear();

	render();
}

void BarycentricsApp::render()
{
	const GfxCapability& caps = Gfx_GetCapability();

	Mat4 matView = m_interpolatedCamera.buildViewMatrix();
	Mat4 matProj = m_interpolatedCamera.buildProjMatrix(caps.projectionFlags);

	Constants constants;
	constants.matView = matView.transposed();
	constants.matProj = matProj.transposed();
	constants.matViewProj = (matView * matProj).transposed();
	constants.matWorld = m_worldTransform.transposed();
	constants.cameraPos = Vec4(m_interpolatedCamera.getPosition());

	Gfx_UpdateBuffer(m_ctx, m_constantBuffer, &constants, sizeof(constants));

	GfxPassDesc passDesc;
	passDesc.flags = GfxPassFlags::ClearAll;
	passDesc.clearColors[0] = ColorRGBA8(11, 22, 33);
	Gfx_BeginPass(m_ctx, passDesc);

	Gfx_SetViewport(m_ctx, GfxViewport(m_window->getSize()));
	Gfx_SetScissorRect(m_ctx, m_window->getSize());

	Gfx_SetDepthStencilState(m_ctx, m_depthStencilStates.writeLessEqual);

	if (m_valid)
	{
		TimingScope<double, 60> timingScope(m_stats.cpuWorld);
		GfxTimerScope gpuTimerScopeWorld(m_ctx, Timestamp_World);

		Gfx_SetBlendState(m_ctx, m_blendStates.opaque);

		switch (m_mode)
		{
		case Mode::Indexed:
			Gfx_SetTechnique(m_ctx, m_techniqueIndexed[m_useTexture].get());
			break;
		case Mode::NonIndexed:
			Gfx_SetTechnique(m_ctx, m_techniqueNonIndexed[m_useTexture].get());
			break;
		case Mode::GeometryShader:
			Gfx_SetTechnique(m_ctx, m_techniqueGeometryShader[m_useTexture].get());
			break;
		case Mode::Manual:
			Gfx_SetTechnique(m_ctx, m_techniqueManual[m_useTexture].get());
			break;
		case Mode::PassthroughGS:
			Gfx_SetTechnique(m_ctx, m_techniquePassthroughGS[m_useTexture].get());
			break;
		case Mode::NativeAMD:
			Gfx_SetTechnique(m_ctx, m_techniqueNativeAMD[m_useTexture].get());
			break;
		default:
			RUSH_LOG_ERROR("Rendering mode '%s' not implemented", toString(m_mode));
		}

		if (m_mode != Mode::NonIndexed)
		{
			Gfx_SetVertexStream(m_ctx, 0, m_vertexBuffer);
			Gfx_SetIndexStream(m_ctx, m_indexBuffer);
		}

		Gfx_SetConstantBuffer(m_ctx, 0, m_constantBuffer);

		Gfx_SetStorageBuffer(m_ctx, 0, m_vertexBuffer);
		Gfx_SetStorageBuffer(m_ctx, 1, m_indexBuffer);

		Gfx_SetConstantBuffer(m_ctx, 1, m_defaultMaterial.constantBuffer);
		Gfx_SetTexture(m_ctx, GfxStage::Pixel, 0, m_defaultMaterial.albedoTexture, m_samplerStates.anisotropicWrap);

		if (m_mode == Mode::NonIndexed)
		{
			Gfx_Draw(m_ctx, 0, m_indexCount);
		}
		else
		{
			Gfx_DrawIndexed(m_ctx, m_indexCount, 0, 0, m_vertexCount);
		}
	}

	// Draw UI on top
	if (m_showUI)
	{
		GfxTimerScope gpuTimerScopeUI(m_ctx, Timestamp_UI);
		TimingScope<double, 60> timingScope(m_stats.cpuUI);

		Gfx_SetBlendState(m_ctx, m_blendStates.lerp);
		Gfx_SetDepthStencilState(m_ctx, m_depthStencilStates.disable);

		m_prim->begin2D(m_window->getSize());

		m_font->setScale(2.0f);

		Vec2 textOrigin = Vec2(10.0f);
		Vec2 pos = textOrigin;
		pos = m_font->draw(m_prim, pos, m_statusString.c_str());
		pos = m_font->draw(m_prim, pos, "\n");
		pos.x = textOrigin.x;

		char tempString[1024];

		pos = m_font->draw(m_prim, pos, "Mode: ");
		pos = m_font->draw(m_prim, pos, toString(m_mode), ColorRGBA8(255, 255, 64));
		pos = m_font->draw(m_prim, pos, "\n");
		pos.x = textOrigin.x;
		
		const GfxStats& stats = Gfx_Stats();
		sprintf_s(tempString,
			"Textured: %d\n"
			"Draw calls: %d\n"
			"Vertices: %d\n"
			"GPU total: %.2f ms\n"
			"> World: %.2f\n"
			"> UI: %.2f\n"
			"CPU time: %.2f ms\n"
			"> World: %.2f ms\n"
			"> UI: %.2f ms",
			int(m_useTexture),
			stats.drawCalls,
			stats.vertices,
			m_stats.gpuTotal.get() * 1000.0f,
			m_stats.gpuWorld.get() * 1000.0f,
			m_stats.gpuUI.get() * 1000.0f,
			m_stats.cpuTotal.get() * 1000.0f,
			m_stats.cpuWorld.get() * 1000.0f,
			m_stats.cpuUI.get() * 1000.0f);
		pos = m_font->draw(m_prim, pos, tempString);
		pos.x = textOrigin.x;

		pos = Vec2(10, m_window->getSizeFloat().y - 30);
		pos = m_font->draw(m_prim, pos, "Controls: number keys to change modes, 'T' to toggle texturing, 'H' to hide UI");

		m_prim->end2D();
	}
	else
	{
		GfxTimerScope gpuTimerScopeUI(m_ctx, Timestamp_UI);
		m_stats.cpuUI.add(0);
	}

	Gfx_EndPass(m_ctx);
}

bool BarycentricsApp::loadModel(const char* filename)
{
	Log::message("Loading model '%s'", filename);

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string errors;

	std::string directory = directoryFromFilename(filename);

	bool loaded = tinyobj::LoadObj(shapes, materials, errors, filename, directory.c_str());
	if (!loaded)
	{
		Log::error("Could not load model from '%s'\n%s\n", filename, errors.c_str());
		return false;
	}

	std::vector<Vertex> vertices;
	std::vector<u32> indices;

	m_boundingBox.expandInit();

	for (const auto& shape : shapes)
	{
		u32 firstVertex = (u32)vertices.size();
		const auto& mesh = shape.mesh;

		const u32 vertexCount = (u32)mesh.positions.size() / 3;

		const bool haveTexcoords = !mesh.texcoords.empty();

		for (u32 i = 0; i < vertexCount; ++i)
		{
			Vertex v;

			v.position.x = mesh.positions[i * 3 + 0];
			v.position.y = mesh.positions[i * 3 + 1];
			v.position.z = mesh.positions[i * 3 + 2];

			m_boundingBox.expand(v.position);

			if (haveTexcoords)
			{
				v.texcoord.x = mesh.texcoords[i * 2 + 0];
				v.texcoord.y = mesh.texcoords[i * 2 + 1];
			}
			else
			{
				v.texcoord = Vec2(0.0f);
			}

			v.position.x = -v.position.x;

			vertices.push_back(v);
		}

		const u32 triangleCount = (u32)mesh.indices.size() / 3;
		for (u32 triangleIt = 0; triangleIt < triangleCount; ++triangleIt)
		{
			indices.push_back(mesh.indices[triangleIt * 3 + 0] + firstVertex);
			indices.push_back(mesh.indices[triangleIt * 3 + 2] + firstVertex);
			indices.push_back(mesh.indices[triangleIt * 3 + 1] + firstVertex);
		}
	}

	m_vertexCount = (u32)vertices.size();
	m_indexCount = (u32)indices.size();

	meshopt_optimizeVertexCache<u32>(indices.data(), indices.data(), m_indexCount, m_vertexCount);

	GfxBufferDesc vbDesc(GfxBufferFlags::Vertex | GfxBufferFlags::Storage, GfxFormat_Unknown, m_vertexCount, sizeof(Vertex));
	m_vertexBuffer = Gfx_CreateBuffer(vbDesc, vertices.data());

	GfxBufferDesc ibDesc(GfxBufferFlags::Index | GfxBufferFlags::Storage, GfxFormat_R32_Uint, m_indexCount, 4);
	m_indexBuffer = Gfx_CreateBuffer(ibDesc, indices.data());

	return true;
}


bool BarycentricsApp::loadTunnelTestModel()
{
	Log::message("Creating tunnel test model");

	std::vector<Vertex> vertices;
	std::vector<u32> indices;

	m_boundingBox.expandInit();

	const float near = 0.0f;
	const float far = 100.0f;
	const float radius = 1.0f;
	const float uscale = 10.0f;

	const u32 circleVertexCount = 50;

	// Last vertices have unique tex coords so need them
	for (u32 i = 0; i <= circleVertexCount; ++i)
	{
		float n = static_cast<float>(i) / static_cast<float>(circleVertexCount);

		Vertex v;
		v.position.x = radius * std::sin(Rush::TwoPi * n);
		v.position.y = radius * std::cos(Rush::TwoPi * n);
		v.texcoord.x = n * uscale;

		// Near vertex
		v.position.z = near;
		v.texcoord.y = near;
		m_boundingBox.expand(v.position);
		vertices.push_back(v);

		// Far vertex
		v.position.z = far;
		v.texcoord.y = far;
		m_boundingBox.expand(v.position);
		vertices.push_back(v);
	}

	m_vertexCount = (u32)vertices.size();
	
	// One quad (connecting near/far pair of vertices) per segment
	for (u32 i = 0; i < circleVertexCount; ++i)
	{
		int i0 = (2*i + 0);
		int i1 = (2*i + 1);
		int i2 = (2*i + 2);
		int i3 = (2*i + 3);

		indices.push_back(i0);
		indices.push_back(i1);
		indices.push_back(i2);

		indices.push_back(i2);
		indices.push_back(i1);
		indices.push_back(i3);
	}
		
	m_indexCount = (u32)indices.size();

	//meshopt_optimizeVertexCache<u32>(indices.data(), indices.data(), m_indexCount, m_vertexCount);

	GfxBufferDesc vbDesc(GfxBufferFlags::Vertex | GfxBufferFlags::Storage, GfxFormat_Unknown, m_vertexCount, sizeof(Vertex));
	m_vertexBuffer = Gfx_CreateBuffer(vbDesc, vertices.data());

	GfxBufferDesc ibDesc(GfxBufferFlags::Index | GfxBufferFlags::Storage, GfxFormat_R32_Uint, m_indexCount, 4);
	m_indexBuffer = Gfx_CreateBuffer(ibDesc, indices.data());

	return true;
}