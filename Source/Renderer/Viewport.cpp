#include "Renderer/Viewport.h"
#include "Engine/Engine.h"
#include "Input/InputHandler.h"
#include "Math/MathCommon.h"
#include "Math/Vector.h"

#include "Pipeline/D3D11.h"
#include "Pipeline/Scanline.h"

Viewport::Viewport(const int32 inWidth, const int32 inHeight)
{
	m_camera           = std::make_shared<Camera>();
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
	return m_camera.get();
}

void Viewport::resetView() const
{
	m_camera->setTranslation(g_defaultCameraTranslation);
	m_camera->setRotation(rotf());

	m_camera->m_target = vec3f::zeroVector();
	m_camera->computeViewProjectionMatrix();
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
	m_renderPipeline = std::make_shared<D3D11RenderPipeline>();
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

void Viewport::updateSceneGeometry() const
{
	std::vector<float> vertArray;
	std::vector<int32> indexArray;
	int32 vertexCount = 0;

	for (const auto& mesh : g_meshes)
	{
		std::vector<Triangle>* tris = mesh->getTriangles();
		for (auto& tri : *tris)
		{
			Vertex* v0 = &tri.v0;
			vertArray.push_back(v0->position.x);
			vertArray.push_back(v0->position.y);
			vertArray.push_back(v0->position.z);
			vertArray.push_back(-v0->normal.x);
			vertArray.push_back(-v0->normal.y);
			vertArray.push_back(-v0->normal.z);

			Vertex* v1 = &tri.v1;
			vertArray.push_back(v1->position.x);
			vertArray.push_back(v1->position.y);
			vertArray.push_back(v1->position.z);
			vertArray.push_back(-v1->normal.x);
			vertArray.push_back(-v1->normal.y);
			vertArray.push_back(-v1->normal.z);

			Vertex* v2 = &tri.v2;
			vertArray.push_back(v2->position.x);
			vertArray.push_back(v2->position.y);
			vertArray.push_back(v2->position.z);
			vertArray.push_back(-v2->normal.x);
			vertArray.push_back(-v2->normal.y);
			vertArray.push_back(-v2->normal.z);

			vertexCount += 3;
		}
	}

	m_renderPipeline->setVertexData(vertArray.data(), vertArray.size() * sizeof(float), vertexCount);
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
		getSize().toString()
	);
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
