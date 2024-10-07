#include "Renderer/Viewport.h"
#include "Engine/Engine.h"
#include "Math/MathCommon.h"
#include "Math/Vector.h"
#include "Engine/ObjectManager.h"

#include "Pipeline/D3D11.h"
#include "Pipeline/Scanline.h"

Viewport::Viewport(const int32 inWidth, const int32 inHeight)
{
	m_camera           = g_objectManager.createObject<Camera>();
	m_camera->m_width  = inWidth;
	m_camera->m_height = inHeight;

	m_grid = std::make_unique<Grid>(8, 4.0f);

	createRenderPipeline();

	// Construct default render settings
	m_settings = RenderSettings();
}

Viewport::~Viewport()
{
	if (m_renderPipeline != nullptr)
	{
		m_renderPipeline->shutdown();
	}
	else
	{
		LOG_ERROR("Render Pipeline is not set. (Renderer::~Renderer)")
	}
}

void Viewport::resize(const int32 inWidth, const int32 inHeight) const
{
	m_camera->m_width  = inWidth;
	m_camera->m_height = inHeight;
	m_renderPipeline->resize(inWidth, inHeight);
}

int32 Viewport::getWidth() const
{
	return m_camera->m_width;
}

int32 Viewport::getHeight() const
{
	return m_camera->m_height;
}

vec2f Viewport::getSize() const
{
	return {static_cast<float>(m_camera->m_width), static_cast<float>(m_camera->m_height)};
}

float Viewport::getAspect() const
{
	return static_cast<float>(m_camera->m_width) / static_cast<float>(m_camera->m_height);
}

Camera* Viewport::getCamera() const
{
	return m_camera;
}

void Viewport::resetView() const
{
	m_camera->setDefault();
}

void Viewport::draw()
{
	if (m_renderPipeline != nullptr)
	{
		// Transfer render settings
		m_renderPipeline->setRenderSettings(&m_settings);

		// Draw all geometry
		m_renderPipeline->beginDraw();

		// Called prior to drawing geometry, draws the world grid
		m_renderPipeline->drawGrid(m_grid.get());

		// Draws all mesh geometry to the framebuffer
		m_renderPipeline->draw();

		// Called after drawing geometry
		m_renderPipeline->endDraw();
	}
	else
	{
		LOG_ERROR("Render Pipeline is not set. (Renderer::beginDraw)")
	}
}

bool Viewport::createRenderPipeline()
{
	// Construct the render pipeline
#ifdef PENG_HARDWARE_ACCELERATION
	m_renderPipeline = std::make_shared<D3D11RenderPipeline>();
#else
	m_renderPipeline = std::make_shared<ScanlineRenderPipeline>();
#endif

	// TODO: For some reason normals need to be flipped in D3D11
	if (dynamic_cast<D3D11RenderPipeline*>(m_renderPipeline.get()))
	{
		m_flipNormals = true;
	}
	// TODO

	return true;
}

bool Viewport::initRenderPipeline(void* windowHandle) const
{
	if (!m_renderPipeline->init(windowHandle))
	{
		return false;
	}

	m_renderPipeline->setViewData(m_camera->getViewData());

	return true;
}

IRenderPipeline* Viewport::getRenderPipeline() const
{
	return m_renderPipeline.get();
}

void Viewport::updateSceneGeometry(const std::vector<IRenderable*>& renderables) const
{
	std::vector<float> vertexArray;
	std::vector<int32> indexArray;
	int32 vertexCount = 0;

	for (const auto& renderable : renderables)
	{
		Mesh* mesh                  = renderable->getMesh();
		transf transform            = renderable->getTransform();
		std::vector<Triangle>* tris = mesh->getTriangles();
		for (auto& tri : *tris)
		{
			Vertex* v0 = &tri.v0;
			vertexArray.push_back(v0->position.x);
			vertexArray.push_back(v0->position.y);
			vertexArray.push_back(v0->position.z);
			vertexArray.push_back(v0->normal.x);
			vertexArray.push_back(v0->normal.y);
			vertexArray.push_back(v0->normal.z);
			vertexArray.push_back(v0->texCoord.x);
			vertexArray.push_back(v0->texCoord.y);

			Vertex* v1 = &tri.v1;
			vertexArray.push_back(v1->position.x);
			vertexArray.push_back(v1->position.y);
			vertexArray.push_back(v1->position.z);
			vertexArray.push_back(v1->normal.x);
			vertexArray.push_back(v1->normal.y);
			vertexArray.push_back(v1->normal.z);
			vertexArray.push_back(v1->texCoord.x);
			vertexArray.push_back(v1->texCoord.y);

			Vertex* v2 = &tri.v2;
			vertexArray.push_back(v2->position.x);
			vertexArray.push_back(v2->position.y);
			vertexArray.push_back(v2->position.z);
			vertexArray.push_back(v2->normal.x);
			vertexArray.push_back(v2->normal.y);
			vertexArray.push_back(v2->normal.z);
			vertexArray.push_back(v2->texCoord.x);
			vertexArray.push_back(v2->texCoord.y);

			vertexCount += 3;
		}
	}

	m_renderPipeline->setVertexData(vertexArray.data(), vertexArray.size() * sizeof(float), vertexCount);
}

void Viewport::updateSceneCamera() const
{
	m_camera->computeViewProjectionMatrix();
	m_renderPipeline->setViewData(m_camera->getViewData());
}

void Viewport::formatDebugText()
{
	const Engine* engine = Engine::getInstance();
	m_debugText          = std::format(
		"Stats\n"
		"FPS: {}\n"
		"Size: {}\n",
		engine->getFps(),
		getSize().toString());
}

std::string Viewport::getDebugText() const
{
	return m_debugText;
}

bool Viewport::getShowDebugText() const
{
	return m_showDebugText;
}

void Viewport::toggleShowDebugText()
{
	m_showDebugText = !m_showDebugText;
}
