// ReSharper disable CppClangTidyClangDiagnosticLanguageExtensionToken
#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11.lib")		// direct3D library
#pragma comment(lib, "dxgi.lib")		// directx graphics interface
#pragma comment(lib, "d3dcompiler.lib") // shader compiler

#include <filesystem>
#include <map>

#include <Windows.h>
#include <wrl/client.h>
#include <comdef.h>
#include <atlstr.h>

#include <d3d11_1.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <directxcolors.h>

#include "RenderPipeline.h"

#include "Core/String.h"

#include "Renderer/Shader.h"

using Microsoft::WRL::ComPtr;

inline const char* formatHResult(const HRESULT err)
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
	ComPtr<ID3D11Device> m_device                     = nullptr;
	ComPtr<ID3D11DeviceContext> m_deviceContext       = nullptr;
	ID3D11Texture2D* m_frameBuffer                    = nullptr;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView = nullptr;
	ID3D11InputLayout* m_inputLayout                  = nullptr;

	std::map<const char*, Shader*> m_shaders;
	std::shared_ptr<VertexShader> m_vertexShader = nullptr;
	std::shared_ptr<PixelShader> m_pixelShader   = nullptr;

public:
	~D3D11RenderPipeline() override = default;

	/** https://walbourn.github.io/anatomy-of-direct3d-11-create-device/ **/
	/** https://antongerdelan.net/opengl/d3d11.html **/
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
		                                   D3D11_SDK_VERSION, &m_device, &m_featureLevel, &m_deviceContext);
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create create device.")
			return false;
		}

		// Obtain DXGI Factory from the device, since we used nullptr for the pAdapter argument of D3D11CreateDevice
		ComPtr<IDXGIFactory1> dxgiFactory = nullptr;
		ComPtr<IDXGIDevice> dxgiDevice    = nullptr;

		result = m_device.As(&dxgiDevice);
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

		result = dxgiFactory->CreateSwapChain(m_device.Get(), &swapChainDesc, &m_swapChain);
		if (SUCCEEDED(result))
		{
			m_initialized = true;
		}
		else
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create swap chain ({}).", formatHResult(result))
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

		result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_frameBuffer));
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create backbuffer ({}).", formatHResult(result))
			return false;
		}

		result = m_device->CreateRenderTargetView(m_frameBuffer, nullptr, &m_renderTargetView);
		m_frameBuffer->Release();
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create render target view ({}).", formatHResult(result))
			return false;
		}

		// Create shaders

		result = createShader("VTX", "VertexShader.hlsl", EShaderType::VertexShader);
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create vertex shader ({}).", formatHResult(result))
			return false;
		}
		result = createShader("PXL", "PixelShader.hlsl", EShaderType::PixelShader);
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create pixel shader ({}).", formatHResult(result))
			return false;
		}

		// Create input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{"SV_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		Shader* vertexShader = getVertexShader();
		result               = m_device->CreateInputLayout(
			inputElementDesc,
			ARRAYSIZE(inputElementDesc),
			vertexShader->getByteCode(),
			vertexShader->getByteCodeSize(),
			&m_inputLayout);

		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed creating input layout ({}).", formatHResult(result));
			return false;
		}

		return true;
	}

	void beginDraw() override
	{
		if (!m_initialized)
		{
			LOG_ERROR("D3D11RenderPipeline::beginDraw(): Pipeline is not initialized.")
			assert(false);
		}
		m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), DirectX::Colors::MidnightBlue);
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
		m_deviceContext->ClearState();

		m_swapChain->Release();
		m_deviceContext->Release();
		m_device->Release();
		m_frameBuffer->Release();
		m_inputLayout->Release();

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

	void setHwnd(const HWND hwnd)
	{
		m_hwnd = hwnd;
	}

	HRESULT createShader(const char* name, const std::string& fileName, EShaderType shaderType)
	{
		std::string profile;
		switch (shaderType)
		{
		case EShaderType::VertexShader:
			{
				m_shaders[name] = new VertexShader();
				profile         = "vs_5_0";
				break;
			}
		case EShaderType::PixelShader:
			{
				m_shaders[name] = new PixelShader();
				profile         = "ps_5_0";
				break;
			}
		default: break;
		}

		Shader* shader = m_shaders[name];

		// TODO: Currently D3DCompileFromFile cannot find relative files, so we have to build the path from the current file.

		auto currentFile    = std::filesystem::path(__FILE__);
		auto currentPath    = currentFile.remove_filename();
		auto shaderFileName = currentPath;
		shaderFileName.append(fileName);
		shader->setFileName(Strings::toString(shaderFileName.c_str()));

		ID3DBlob* blob = nullptr;
		HRESULT result = compileShader(shaderFileName.c_str(), "main", profile.c_str(), &blob);
		if (FAILED(result))
		{
			if (blob)
			{
				blob->Release();
			}
			LOG_ERROR("D3D11RenderPipeline::init(): Failed compiling vertex shader ({}).", formatHResult(result));
			m_shaders.erase(name); // Remove the entry we created
			return result;
		}
		shader->setByteCode(blob->GetBufferPointer(), blob->GetBufferSize());

		return S_OK;
	}

	static HRESULT compileShader(const LPCWSTR fileName, const LPCSTR entryPoint, const LPCSTR profile, ID3DBlob** blob)
	{
		if (!fileName || !entryPoint || !blob)
		{
			return E_INVALIDARG;
		}

		*blob = nullptr;

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
		flags |= D3DCOMPILE_DEBUG;
#endif

		ID3DBlob* shaderBlob = nullptr;
		ID3DBlob* errorBlob  = nullptr;
		HRESULT result       = D3DCompileFromFile(fileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		                                    entryPoint, profile,
		                                    flags, 0, &shaderBlob, &errorBlob);
		if (FAILED(result))
		{
			if (errorBlob)
			{
				const char* errorMessage = (char*)errorBlob->GetBufferPointer();
				LOG_ERROR("D3D11RenderPipeline::compileShader(): Failed to compile shader ({}).", errorMessage)
				errorBlob->Release();
			}
			else
			{
				LOG_ERROR("D3D11RenderPipeline::compileShader(): Shader file not found ({}).",
				          Strings::toString(fileName).c_str())
			}

			if (shaderBlob)
			{
				shaderBlob->Release();
			}

			return result;
		}

		*blob = shaderBlob;

		return result;
	}

	Shader* getVertexShader()
	{
		for (const auto& [k, v] : m_shaders)
		{
			if (v->getResourceType() == EShaderType::VertexShader)
			{
				return v;
			}
		}
		return nullptr;
	}
};
