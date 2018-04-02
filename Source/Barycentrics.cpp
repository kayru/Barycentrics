#include "Barycentrics.h"

#include <Rush/UtilFile.h>
#include <Rush/UtilLog.h>
#include <Rush/MathTypes.h>

#pragma warning(push)
#pragma warning(disable: 4996)
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#pragma warning(pop)

#include <algorithm>

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
	vfDefaultDesc.add(0, GfxVertexFormatDesc::DataType::Float3, GfxVertexFormatDesc::Semantic::Normal, 0);
	vfDefaultDesc.add(0, GfxVertexFormatDesc::DataType::Float2, GfxVertexFormatDesc::Semantic::Texcoord, 0);

	GfxVertexFormatDesc vfEmptyDesc;

	GfxVertexShaderRef vsIndexed;
	vsIndexed.takeover(Gfx_CreateVertexShader(shaderFromFile("Shaders/ModelIndexed.vert.spv")));

	{
		GfxVertexShaderRef vs;
		vs.takeover(Gfx_CreateVertexShader(shaderFromFile("Shaders/Model.vert.spv")));

		GfxPixelShaderRef ps;
		ps.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/Model.frag.spv")));

		GfxVertexFormatRef vf;
		vf.takeover(Gfx_CreateVertexFormat(vfEmptyDesc));

		m_techniqueNonIndexed = Gfx_CreateTechnique(GfxTechniqueDesc(ps.get(), vs.get(), vf.get(), &bindings));
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

		m_techniqueGeometryShader = Gfx_CreateTechnique(techniqueDesc);
	}

	{
		GfxPixelShaderRef ps;
		ps.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/ModelIndexed.frag.spv")));

		GfxVertexFormatRef vf;
		vf.takeover(Gfx_CreateVertexFormat(vfDefaultDesc));

		m_techniqueIndexed = Gfx_CreateTechnique(GfxTechniqueDesc(ps.get(), vsIndexed.get(), vf.get(), &bindings));
	}

	{
		GfxVertexShaderRef vs;
		vs.takeover(Gfx_CreateVertexShader(shaderFromFile("Shaders/ModelManual.vert.spv")));

		GfxPixelShaderRef ps;
		ps.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/ModelManual.frag.spv")));

		GfxVertexFormatRef vf;
		vf.takeover(Gfx_CreateVertexFormat(vfDefaultDesc));

		m_techniqueManual = Gfx_CreateTechnique(GfxTechniqueDesc(ps.get(), vs.get(), vf.get(), &bindings));
	}

	if (Gfx_GetCapability().geometryShaderPassthroughNV)
	{
		GfxVertexShaderRef vs;
		vs.takeover(Gfx_CreateVertexShader(shaderFromFile("Shaders/ModelPassthrough.vert.spv")));

		GfxPixelShaderRef ps;
		ps.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/ModelPassthrough.frag.spv")));

		GfxGeometryShaderRef gs;
		gs.takeover(Gfx_CreateGeometryShader(shaderFromFile("Shaders/ModelPassthrough.geom.spv")));

		GfxVertexFormatRef vf;
		vf.takeover(Gfx_CreateVertexFormat(vfDefaultDesc));

		GfxTechniqueDesc techniqueDesc(ps.get(), vs.get(), vf.get(), &bindings);
		techniqueDesc.gs = gs.get();

		m_techniquePassthroughGS = Gfx_CreateTechnique(techniqueDesc);
	}

	if (Gfx_GetCapability().explicitVertexParameterAMD)
	{
		GfxVertexShaderRef vs;
		vs.takeover(Gfx_CreateVertexShader(shaderFromFile("Shaders/ModelNativeAMD.vert.spv")));

		GfxPixelShaderRef ps;
		ps.takeover(Gfx_CreatePixelShader(shaderFromFile("Shaders/ModelNativeAMD.frag.spv")));

		GfxVertexFormatRef vf;
		vf.takeover(Gfx_CreateVertexFormat(vfDefaultDesc));

		m_techniqueNativeAMD = Gfx_CreateTechnique(GfxTechniqueDesc(ps.get(), vs.get(), vf.get(), &bindings));
	}

	GfxBufferDesc cbDescr(GfxBufferFlags::TransientConstant, GfxFormat_Unknown, 1, sizeof(Constants));
	m_constantBuffer = Gfx_CreateBuffer(cbDescr);

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
	}
	else
	{
		m_statusString = "Usage: BarycentricsApp <filename.obj>";
	}

	float aspect = m_window->getAspect();
	float fov = 1.0f;

	m_camera = Camera(aspect, fov, 0.25f, 10000.0f);
	m_camera.lookAt(Vec3(m_boundingBox.m_max) + Vec3(2.0f), m_boundingBox.center());
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
			else if (e.code == Key_4 && m_techniquePassthroughGS.valid())
			{
				m_mode = Mode::PassthroughGS;
			}
			else if (e.code == Key_5 && m_techniqueNativeAMD.valid())
			{
				m_mode = Mode::NativeAMD;
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
			Gfx_SetTechnique(m_ctx, m_techniqueIndexed);
			break;
		case Mode::NonIndexed:
			Gfx_SetTechnique(m_ctx, m_techniqueNonIndexed);
			break;
		case Mode::GeometryShader:
			Gfx_SetTechnique(m_ctx, m_techniqueGeometryShader);
			break;
		case Mode::Manual:
			Gfx_SetTechnique(m_ctx, m_techniqueManual);
			break;
		case Mode::PassthroughGS:
			Gfx_SetTechnique(m_ctx, m_techniquePassthroughGS);
			break;
		case Mode::NativeAMD:
			Gfx_SetTechnique(m_ctx, m_techniqueNativeAMD);
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

		for (const MeshSegment& segment : m_segments)
		{
			GfxTexture texture = m_defaultWhiteTexture;

			const Material& material = (segment.material == 0xFFFFFFFF) ? m_defaultMaterial : m_materials[segment.material];
			if (material.albedoTexture.valid())
			{
				texture = material.albedoTexture.get();
			}

			Gfx_SetConstantBuffer(m_ctx, 1, material.constantBuffer);

			Gfx_SetTexture(m_ctx, GfxStage::Pixel, 0, texture, m_samplerStates.anisotropicWrap);

			if (m_mode == Mode::NonIndexed)
			{
				Gfx_Draw(m_ctx, segment.indexOffset, segment.indexCount);
			}
			else
			{
				Gfx_DrawIndexed(m_ctx, segment.indexCount, segment.indexOffset, 0, m_vertexCount);
			}
		}
	}

	// Draw UI on top
	{
		GfxTimerScope gpuTimerScopeWorld(m_ctx, Timestamp_UI);
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
			"Draw calls: %d\n"
			"Vertices: %d\n"
			"GPU total: %.2f ms\n"
			"> World: %.2f\n"
			"> UI: %.2f\n"
			"CPU time: %.2f ms\n"
			"> World: %.2f ms\n"
			"> UI: %.2f ms",
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

		m_prim->end2D();
	}

	Gfx_EndPass(m_ctx);
}

GfxRef<GfxTexture> BarycentricsApp::loadTexture(const std::string& filename)
{
	auto it = m_textures.find(filename);
	if (it == m_textures.end())
	{
		Log::message("Loading texture '%s'", filename.c_str());

		GfxRef<GfxTexture> texture;
		texture.takeover(textureFromFile(filename.c_str()));

		m_textures.insert(std::make_pair(filename, texture));

		return texture;
	}
	else
	{
		return it->second;
	}
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

	const GfxBufferDesc materialCbDesc(GfxBufferFlags::Constant, GfxFormat_Unknown, 1, sizeof(MaterialConstants));
	for (auto& objMaterial : materials)
	{
		MaterialConstants constants;
		constants.baseColor.x = objMaterial.diffuse[0];
		constants.baseColor.y = objMaterial.diffuse[1];
		constants.baseColor.z = objMaterial.diffuse[2];
		constants.baseColor.w = 1.0f;

		Material material;
		if (!objMaterial.diffuse_texname.empty())
		{
			material.albedoTexture = loadTexture(directory + objMaterial.diffuse_texname);
		}

		{
			u64 constantHash = hashFnv1a64(&constants, sizeof(constants));
			auto it = m_materialConstantBuffers.find(constantHash);
			if (it == m_materialConstantBuffers.end())
			{
				GfxBuffer cb = Gfx_CreateBuffer(materialCbDesc, &constants);
				m_materialConstantBuffers[constantHash].retain(cb);
				material.constantBuffer.retain(cb);
			}
			else
			{
				material.constantBuffer = it->second;
			}
		}

		m_materials.push_back(material);
	}

	{
		MaterialConstants constants;
		constants.baseColor = Vec4(1.0f);
		m_defaultMaterial.constantBuffer.takeover(Gfx_CreateBuffer(materialCbDesc, &constants));
		m_defaultMaterial.albedoTexture.retain(m_defaultWhiteTexture);
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
		const bool haveNormals = mesh.positions.size() == mesh.normals.size();

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

			if (haveNormals)
			{
				v.normal.x = mesh.normals[i * 3 + 0];
				v.normal.y = mesh.normals[i * 3 + 1];
				v.normal.z = mesh.normals[i * 3 + 2];
			}
			else
			{
				v.normal = Vec3(0.0);
			}

			v.position.x = -v.position.x;
			v.normal.x = -v.normal.x;

			vertices.push_back(v);
		}

		if (!haveNormals)
		{
			const u32 triangleCount = (u32)mesh.indices.size() / 3;
			for (u32 i = 0; i < triangleCount; ++i)
			{
				u32 idxA = firstVertex + mesh.indices[i * 3 + 0];
				u32 idxB = firstVertex + mesh.indices[i * 3 + 2];
				u32 idxC = firstVertex + mesh.indices[i * 3 + 1];

				Vec3 a = vertices[idxA].position;
				Vec3 b = vertices[idxB].position;
				Vec3 c = vertices[idxC].position;

				Vec3 normal = cross(b - a, c - b);

				normal = normalize(normal);

				vertices[idxA].normal += normal;
				vertices[idxB].normal += normal;
				vertices[idxC].normal += normal;
			}

			for (u32 i = firstVertex; i < (u32)vertices.size(); ++i)
			{
				vertices[i].normal = normalize(vertices[i].normal);
			}
		}

		u32 currentMaterialId = 0xFFFFFFFF;

		const u32 triangleCount = (u32)mesh.indices.size() / 3;
		for (u32 triangleIt = 0; triangleIt < triangleCount; ++triangleIt)
		{
			if (mesh.material_ids[triangleIt] != currentMaterialId || m_segments.empty())
			{
				currentMaterialId = mesh.material_ids[triangleIt];
				m_segments.push_back(MeshSegment());
				m_segments.back().material = currentMaterialId;
				m_segments.back().indexOffset = (u32)indices.size();
				m_segments.back().indexCount = 0;
			}

			indices.push_back(mesh.indices[triangleIt * 3 + 0] + firstVertex);
			indices.push_back(mesh.indices[triangleIt * 3 + 2] + firstVertex);
			indices.push_back(mesh.indices[triangleIt * 3 + 1] + firstVertex);

			m_segments.back().indexCount += 3;
		}
	}

	m_vertexCount = (u32)vertices.size();
	m_indexCount = (u32)indices.size();

	GfxBufferDesc vbDesc(GfxBufferFlags::Vertex | GfxBufferFlags::Storage, GfxFormat_Unknown, m_vertexCount, sizeof(Vertex));
	m_vertexBuffer = Gfx_CreateBuffer(vbDesc, vertices.data());

	GfxBufferDesc ibDesc(GfxBufferFlags::Index | GfxBufferFlags::Storage, GfxFormat_R32_Uint, m_indexCount, 4);
	m_indexBuffer = Gfx_CreateBuffer(ibDesc, indices.data());

	return true;
}
