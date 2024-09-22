// ReSharper disable CppClangTidyClangDiagnosticLanguageExtensionToken
#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <Windows.h>
#include <d3d11_1.h>
#include <directxcolors.h>
#include <wrl/client.h>
#include <comdef.h>
#include <atlstr.h>

#include "RenderPipeline.h"

#include "Framework/Renderer/Shader.h"

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
	bool m_initialized = false;
	HWND m_hwnd        = nullptr;

	int32 m_width  = 0;
	int32 m_height = 0;

	D3D_FEATURE_LEVEL m_featureLevel                  = D3D_FEATURE_LEVEL_11_0;
	ComPtr<IDXGISwapChain> m_swapChain                = nullptr;
	ComPtr<ID3D11Device> m_d3dDevice                  = nullptr;
	ComPtr<ID3D11DeviceContext> m_immediateContext    = nullptr;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView = nullptr;

	std::shared_ptr<VertexShader> m_vertexShader = nullptr;
	std::shared_ptr<PixelShader> m_pixelShader   = nullptr;

public:
	void init() override
	{
		ASSERT(m_hwnd != nullptr, "HWND not set in D3D11 Pipeline.");

		// Create the device
		DWORD createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr, 0,
		                  D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel,
		                  &m_immediateContext);

		// Obtain DXGI Factory from the device, since we used nullptr for the pAdapter argument of D3D11CreateDevice
		IDXGIFactory1* dxgiFactory = nullptr;
		IDXGIDevice* dxgiDevice    = nullptr;
		HRESULT result             = m_immediateContext->QueryInterface(__uuidof(IDXGIDevice),
		                                                    reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(result))
		{
			IDXGIAdapter* adapter = nullptr;
			result                = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(result))
			{
				adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
		else
		{
			LOG_ERROR("Failed to create D3D11 Device: {}", fmt(result))
			return;
		}

		// Create the swap chain
		DXGI_SWAP_CHAIN_DESC swapChainDesc               = {};
		swapChainDesc.BufferCount                        = 1;
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

		dxgiFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
		dxgiFactory->Release();

		// Create a render target view
		ID3D11Texture2D* pBackBuffer = nullptr;
		result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
		if (FAILED(result))
			return;

		result = m_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetView);
		pBackBuffer->Release();
		if (FAILED(result))
			return;

		m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

		// Setup the viewport
		D3D11_VIEWPORT vp;
		vp.Width    = (FLOAT)m_width;
		vp.Height   = (FLOAT)m_height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_immediateContext->RSSetViewports(1, &vp);
	}

	void beginDraw() override
	{
		m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), DirectX::Colors::Black);
		m_swapChain->Present(0, 0);
	}

	void endDraw() override {}

	void shutdown() override
	{
		if (m_immediateContext)
		{
			m_immediateContext->ClearState();
		}
		if (m_swapChain)
		{
			m_swapChain->Release();
		}
		if (m_immediateContext)
		{
			m_immediateContext->Release();
		}
		if (m_d3dDevice)
		{
			m_d3dDevice->Release();
		}
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
