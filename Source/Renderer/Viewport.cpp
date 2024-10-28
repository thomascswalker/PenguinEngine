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

	createRHI();
	// Construct default render settings
	m_settings = RenderSettings();
}

Viewport::~Viewport()
{
	if (m_rhi != nullptr)
	{
		m_rhi->shutdown();
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
	m_rhi->resize(inWidth, inHeight);
}

int32 Viewport::getWidth() const
{
	return m_camera->m_width;
}

int32 Viewport::getHeight() const
{
	return m_camera->m_height;
}

vec2i Viewport::getSize() const
{
	return {m_camera->m_width, m_camera->m_height};
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
	if (m_rhi != nullptr)
	{
		// Transfer render settings
		m_rhi->setRenderSettings(&m_settings);

		// Update camera data
		m_rhi->setViewData(m_camera->getViewData());

		// Draw all geometry
		m_rhi->beginDraw();

		// Called prior to drawing geometry, draws the world grid
		m_rhi->drawGrid(m_grid.get());

		// Draw each renderable object
		m_rhi->draw();

		// Called after drawing geometry
		m_rhi->endDraw();
	}
	else
	{
		LOG_ERROR("Render Pipeline is not set. (Renderer::beginDraw)")
	}
}

bool Viewport::createRHI()
{
	// Construct the render pipeline
#ifdef PENG_HARDWARE_ACCELERATION
	m_rhi = std::make_shared<D3D11RHI>();
#else
	m_rhi = std::make_shared<ScanlineRHI>();
#endif

	return true;
}

bool Viewport::initRHI(void* windowHandle) const
{
	if (!m_rhi->init(windowHandle))
	{
		return false;
	}

	m_rhi->setViewData(m_camera->getViewData());

	return true;
}

IRHI* Viewport::getRHI() const
{
	return m_rhi.get();
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
