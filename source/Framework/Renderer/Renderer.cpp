// ReSharper disable CppClangTidyBugproneNarrowingConversions
// ReSharper disable CppClangTidyClangDiagnosticFloatConversion
// ReSharper disable CppClangTidyClangDiagnosticImplicitIntFloatConversion
#pragma warning(disable : 4244)

#include <thread>

#include "Framework/Renderer/Renderer.h"
#include "Framework/Renderer/Shader.h"
#include "Framework/Renderer/Texture.h"
#include "Framework/Importers/TextureImporter.h"

#include "Pipeline/Scanline.h"

Renderer::Renderer(uint32 inWidth, uint32 inHeight)
{
	m_viewport       = std::make_shared<Viewport>(inWidth, inHeight);
	m_grid           = std::make_unique<Grid>(8, 4.0f);
	m_renderPipeline = std::make_shared<ScanlineRenderPipeline>();
	m_renderPipeline->startup();
	m_settings = RenderSettings();
}

Renderer::~Renderer()
{
	m_renderPipeline->shutdown();
}

void Renderer::resize(const uint32 inWidth, const uint32 inHeight)
{
	m_viewport->resize(inWidth, inHeight);
	m_renderPipeline->resize(inWidth, inHeight);
	m_frameData.resize(inWidth, inHeight);
}

void Renderer::draw()
{
	// Recalculate the view-projection matrix of the camera
	m_viewport->getCamera()->computeViewProjectionMatrix();

	ViewData viewData = m_viewport->getCamera()->getViewData();

	// Transfer camera and viewport data
	m_renderPipeline->setViewData(&viewData);

	// Transfer render settings
	m_renderPipeline->setRenderSettings(&m_settings);

	// Called prior to drawing geometry
	m_renderPipeline->preDraw();

	// Called prior to drawing geometry, draws the world grid
	m_renderPipeline->drawGrid(m_grid.get());

	// Draw all geometry
	m_renderPipeline->draw();

	// Called after drawing geometry
	m_renderPipeline->postDraw();
}
