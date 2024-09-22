#include "Framework/Renderer/Viewport.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Input/InputHandler.h"
#include "Math/MathCommon.h"
#include "Math/Vector.h"

#include "Pipeline/Scanline.h"

Viewport::Viewport(const int32 inWidth, const int32 inHeight)
{
	m_camera           = std::make_shared<Camera>();
	m_camera->m_width  = inWidth;
	m_camera->m_height = inHeight;

	m_grid = std::make_unique<Grid>(8, 4.0f);

	// Construct the render pipeline
	m_renderPipeline = std::make_shared<ScanlineRenderPipeline>();
	m_renderPipeline->init();
	m_renderPipeline->setViewData(m_camera->getViewData());

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
	// Recalculate the view-projection matrix of the camera
	m_camera->computeViewProjectionMatrix();

	if (m_renderPipeline != nullptr)
	{
		// Transfer camera and viewport data
		m_renderPipeline->setViewData(m_camera->getViewData());

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

IRenderPipeline* Viewport::getRenderPipeline() const
{
	return m_renderPipeline.get();
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
