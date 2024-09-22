// ReSharper disable CppClangTidyClangDiagnosticLanguageExtensionToken
#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11.lib")		// direct3D library
#pragma comment(lib, "dxgi.lib")		// directx graphics interface
#pragma comment(lib, "d3dcompiler.lib") // shader compiler

#include <Windows.h>
#include <wrl/client.h>
#include <comdef.h>
#include <atlstr.h>

#include <d3d11_1.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <directxcolors.h>

#include "RenderPipeline.h"

#include "Renderer/Shader.h"

using Microsoft::WRL::ComPtr;

inline const char* fmt(const HRESULT err)
{
	auto comErr    = _com_error(err);
	LPCTSTR errMsg = comErr.ErrorMessage();
	CStringA errMsgA(errMsg);
	const char* errMsgP = errMsgA;
	return errMsgP;
}

// https://gist.github.com/d7samurai/261c69490cce0620d0bfc93003cd1052
class D3D11RenderPipeline : public IRenderPipeline
{
private:
	bool m_initialized = false;
	HWND m_hwnd        = nullptr;

	int32 m_width  = 0;
	int32 m_height = 0;

	D3D_FEATURE_LEVEL m_featureLevel                  = D3D_FEATURE_LEVEL_11_0;
	ComPtr<IDXGISwapChain> m_swapChain                = nullptr;
	ComPtr<ID3D11Device> m_d3dDevice                  = nullptr;
	ComPtr<ID3D11DeviceContext> m_deviceContext       = nullptr;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView = nullptr;

	std::shared_ptr<VertexShader> m_vertexShader = nullptr;
	std::shared_ptr<PixelShader> m_pixelShader   = nullptr;

public:
	~D3D11RenderPipeline() override = default;

	/** https://walbourn.github.io/anatomy-of-direct3d-11-create-device/ **/
	bool init(void* windowHandle) override
	{
		m_hwnd = (HWND)windowHandle;
		ASSERT(m_hwnd != nullptr, "D3D11RenderPipeline::init(): HWND not set.");

		// Create the device
		DWORD createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr, 0,
		                                   D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_deviceContext);
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create create device.")
			return false;
		}

		// Obtain DXGI Factory from the device, since we used nullptr for the pAdapter argument of D3D11CreateDevice
		ComPtr<IDXGIFactory1> dxgiFactory = nullptr;
		ComPtr<IDXGIDevice> dxgiDevice    = nullptr;

		result = m_d3dDevice.As(&dxgiDevice);
		if (SUCCEEDED(result))
		{
			IDXGIAdapter* adapter = nullptr;
			result                = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(result))
			{
				result = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
				if (FAILED(result))
				{
					LOG_ERROR("D3D11RenderPipeline::init(): Failed to get adapter parent.")
					return false;
				}
				adapter->Release();
			}
			dxgiDevice->Release();
		}
		else
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to obtain DXGI Factory.")
			return false;
		}

		// Create the swap chain
		DXGI_SWAP_CHAIN_DESC swapChainDesc               = {};
		swapChainDesc.BufferCount                        = 1; // 1 Front and back buffer
		swapChainDesc.BufferDesc.Width                   = m_width;
		swapChainDesc.BufferDesc.Height                  = m_height;
		swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow                       = m_hwnd;
		swapChainDesc.SampleDesc.Count                   = 1;
		swapChainDesc.SampleDesc.Quality                 = 0;
		swapChainDesc.Windowed                           = TRUE;

		result = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &swapChainDesc, &m_swapChain);
		if (SUCCEEDED(result))
		{
			m_initialized = true;
		}
		else
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create swap chain.")
			return false;
		}

		result = dxgiFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create associate HWND {} with DXGI Factory.",
			          (int32)m_hwnd)
			return false;
		}
		dxgiFactory->Release();

		// Create a render target view
		ID3D11Texture2D* pBackBuffer = nullptr;
		result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create backbuffer.")
			return false;
		}

		result = m_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetView);
		pBackBuffer->Release();
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create render target view.")
			return false;
		}

		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

		// Setup the viewport
		D3D11_VIEWPORT vp;
		vp.Width    = (FLOAT)g_defaultViewportWidth;
		vp.Height   = (FLOAT)g_defaultViewportHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_deviceContext->RSSetViewports(1, &vp);

		return true;
	}

	void beginDraw() override
	{
		if (!m_initialized)
		{
			LOG_ERROR("D3D11RenderPipeline::beginDraw(): Pipeline is not initialized.")
			assert(false);
		}
		m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), DirectX::Colors::Black);
		HRESULT result = m_swapChain->Present(0, 0);
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to present swap chain.")
		}
	}

	void draw() override {}

	void endDraw() override {}

	void shutdown() override
	{
		if (m_deviceContext)
		{
			m_deviceContext->ClearState();
		}
		if (m_swapChain)
		{
			m_swapChain->Release();
		}
		if (m_deviceContext)
		{
			m_deviceContext->Release();
		}
		if (m_d3dDevice)
		{
			m_d3dDevice->Release();
		}
		m_initialized = false;
	}

	void resize(int32 width, int32 height) override
	{
		m_width  = width;
		m_height = height;
	}

	void drawGrid(Grid* grid) override {}
	void drawLine(const vec3f& inA, const vec3f& inB, const Color& color) override {}

	uint8* getFrameData() override
	{
		return nullptr;
	}

	void setViewData(ViewData* newViewData) override {}
	void setRenderSettings(RenderSettings* newRenderSettings) override {}

	void setHwnd(HWND hwnd)
	{
		m_hwnd = hwnd;
	}
};
