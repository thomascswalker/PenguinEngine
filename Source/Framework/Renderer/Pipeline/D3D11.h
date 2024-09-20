#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <d3d11.h>
#include <d3dcompiler.h>

#include "RenderPipeline.h"

// https://gist.github.com/d7samurai/261c69490cce0620d0bfc93003cd1052
class D3D11RenderPipeline : public IRenderPipeline
{
	int32 m_width  = 0;
	int32 m_height = 0;

	D3D_FEATURE_LEVEL m_featureLevels[]  = {};
	DXGI_SWAP_CHAIN_DESC m_swapChainDesc = {};
	IDXGISwapChain* m_swapChain          = nullptr;
	ID3D11Device* m_device               = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;

public:
	void startup() override
	{
		m_featureLevels = {D3D_FEATURE_LEVEL_11_0};

		m_swapChainDesc.BufferDesc.Width  = m_width;
		m_swapChainDesc.BufferDesc.Height = m_height;
		m_swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		m_swapChainDesc.SampleDesc.Count  = 1;
		m_swapChainDesc.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		m_swapChainDesc.BufferCount       = 2;
		m_swapChainDesc.OutputWindow      = nullptr;
		m_swapChainDesc.Windowed          = FALSE;
		m_swapChainDesc.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		D3D11CreateDeviceAndSwapChain(nullptr, // Adapter
		                              D3D_DRIVER_TYPE_HARDWARE, // Driver type
		                              nullptr, // Software
		                              D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG, // Flags
		                              m_featureLevels, // Feature levels pointer
		                              ARRAYSIZE(m_featureLevels), // Feature levels size
		                              D3D11_SDK_VERSION, // SDK version
		                              &m_swapChainDesc, // SwapChain description
		                              &m_swapChain, // SwapChain
		                              &m_device, // Device
		                              nullptr, // Feature level pointer
		                              &m_deviceContext); // Device context
	}

	void preDraw() override;
	void draw() override;
	void postDraw() override;
	void shutdown() override;

	void resize(int32 width, int32 height) override
	{
		m_width  = width;
		m_height = height;
	}

	void drawGrid(Grid* grid) override;
	void drawLine(const vec3f& inA, const vec3f& inB, const Color& color) override;
	uint8* getFrameData() override;
	void setViewData(ViewData* newViewData) override;
	void setRenderSettings(RenderSettings* newRenderSettings) override;
};
