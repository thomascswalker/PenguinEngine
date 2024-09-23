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

struct MVPMatrix
{
	DirectX::XMMATRIX mvp;
};

class D3D11VertexShader : public VertexShader
{
public:
	ID3D11VertexShader* m_shaderPtr = nullptr;

	~D3D11VertexShader() override
	{
		if (m_shaderPtr)
		{
			m_shaderPtr->Release();
		}
	}

	ID3D11VertexShader* getPipelineShader() const
	{
		return m_shaderPtr;
	}
};

class D3D11PixelShader : public PixelShader
{
public:
	ID3D11PixelShader* m_shaderPtr = nullptr;

	~D3D11PixelShader() override
	{
		if (m_shaderPtr)
		{
			m_shaderPtr->Release();
		}
	}

	ID3D11PixelShader* getPipelineShader() const
	{
		return m_shaderPtr;
	}
};

// https://gist.github.com/d7samurai/261c69490cce0620d0bfc93003cd1052
class D3D11RenderPipeline : public IRenderPipeline
{
	bool m_initialized = false;
	HWND m_hwnd        = nullptr;

	int32 m_width  = 0;
	int32 m_height = 0;

	/** Boilerplate D3D11 **/

	D3D_FEATURE_LEVEL m_featureLevel            = D3D_FEATURE_LEVEL_11_0;
	ComPtr<IDXGISwapChain> m_swapChain          = nullptr;
	ComPtr<ID3D11Device> m_device               = nullptr;
	ComPtr<ID3D11DeviceContext> m_deviceContext = nullptr;
	ID3D11Texture2D* m_frameBuffer              = nullptr;
	ID3D11RenderTargetView* m_renderTargetView  = nullptr;

	/** Vertex & Index Buffer **/

	ID3D11Buffer* m_vertexBuffer = nullptr;
	float* m_vertexDataArray     = nullptr;
	size_t m_vertexDataSize      = 0;
	uint32 m_vertexStride        = 3 * sizeof(float); // XYZ * 4 = 12
	uint32 m_vertexOffset        = 0;
	uint32 m_vertexCount         = 0;

	ID3D11Buffer* m_indexBuffer = nullptr;
	uint32* m_indexDataArray    = nullptr;
	size_t m_indexDataSize      = 0;
	uint32 m_indexStride        = 3 * sizeof(int32);
	uint32 m_indexOffset        = 0;
	uint32 m_indexCount         = 0;

	/** Shaders **/

	ID3D11InputLayout* m_inputLayout = nullptr;
	std::map<const char*, Shader*> m_shaders;
	D3D11VertexShader* m_vertexShader = nullptr;
	D3D11PixelShader* m_pixelShader   = nullptr;

	/** Camera **/
	ComPtr<ID3D11Buffer> m_cameraBuffer = nullptr;
	float* m_cameraMvp                  = nullptr;

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
		swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
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
			{"SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		result = m_device->CreateInputLayout(
			inputElementDesc,
			ARRAYSIZE(inputElementDesc),
			m_vertexShader->getByteCode(),
			m_vertexShader->getByteCodeSize(),
			&m_inputLayout);

		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed creating input layout ({}).", formatHResult(result));
			return false;
		}

		// https://samulinatri.com/blog/direct3d-11-constant-buffer-tutorial
		D3D11_BUFFER_DESC cameraBufferDesc;
		cameraBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
		cameraBufferDesc.ByteWidth           = 16 * sizeof(float);
		cameraBufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		cameraBufferDesc.CPUAccessFlags      = 0;
		cameraBufferDesc.MiscFlags           = 0;
		cameraBufferDesc.StructureByteStride = 0;

		result = m_device->CreateBuffer(&cameraBufferDesc, nullptr, &m_cameraBuffer);
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed creating the camera constant buffer ({}).",
			          formatHResult(result));
			return false;
		}
		m_deviceContext->VSSetConstantBuffers(0, 1, m_cameraBuffer.GetAddressOf());

		// Always 4x4 float matrix
		m_cameraMvp = PlatformMemory::malloc<float>(16 * sizeof(float));

		return true;
	}

	void beginDraw() override
	{
		if (!m_initialized)
		{
			LOG_ERROR("D3D11RenderPipeline::beginDraw(): Pipeline is not initialized.")
			assert(false);
		}
		m_deviceContext->ClearRenderTargetView(m_renderTargetView, DirectX::Colors::Black);

		// Create the viewport
		RECT winRect;
		GetClientRect(m_hwnd, &winRect);
		D3D11_VIEWPORT viewport = {
			0.0f, 0.0f, (FLOAT)(winRect.right - winRect.left), (FLOAT)(winRect.bottom - winRect.top), 0.0f, 1.0f
		};
		m_deviceContext->RSSetViewports(1, &viewport);

		// Set culling
		ID3D11RasterizerState* rasterState;
		D3D11_RASTERIZER_DESC rasterDesc{};
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		HRESULT result      = m_device->CreateRasterizerState(&rasterDesc, &rasterState);
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::beginDraw(): Failed creating rasterizer state ({}).",
			          formatHResult(result));
			return;
		}
		m_deviceContext->RSSetState(rasterState);

		// Set the render target
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

		// Associate the vertex buffer with the device context
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_deviceContext->IASetInputLayout(m_inputLayout);
		m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_vertexStride, &m_vertexOffset);
		m_deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set the vertex and pixel shaders on the device context
		if (m_vertexShader->m_shaderPtr)
		{
			m_deviceContext->VSSetShader(m_vertexShader->m_shaderPtr, nullptr, 0);
		}
		if (m_pixelShader->m_shaderPtr)
		{
			m_deviceContext->PSSetShader(m_pixelShader->m_shaderPtr, nullptr, 0);
		}
	}

	void draw() override
	{
		m_deviceContext->DrawIndexed(m_indexCount, 0, 0);
		HRESULT result = m_swapChain->Present(1, 0);
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to present swap chain.")
			assert(false);
		}
	}

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

	void setViewData(ViewData* newViewData) override
	{
		// auto transposed      = newViewData->viewProjectionMatrix.getTranspose();
		const float* mvpData = (float*)&newViewData->viewProjectionMatrix.m;
		MVPMatrix mvp;
		mvp.mvp = DirectX::XMMATRIX(mvpData);
		m_deviceContext->UpdateSubresource(m_cameraBuffer.Get(), 0, nullptr, &mvp, 0, 0);
	}

	void setRenderSettings(RenderSettings* newRenderSettings) override {}

	void setHwnd(const HWND hwnd)
	{
		m_hwnd = hwnd;
	}

	HRESULT createShader(const char* name, const std::string& fileName, EShaderType shaderType)
	{
		Shader* shader;
		std::string profile;
		switch (shaderType)
		{
		case EShaderType::VertexShader:
			{
				m_vertexShader = new D3D11VertexShader();
				shader         = m_vertexShader;
				profile        = "vs_5_0";
				break;
			}
		case EShaderType::PixelShader:
			{
				m_pixelShader = new D3D11PixelShader();
				shader        = m_pixelShader;
				profile       = "ps_5_0";
				break;
			}
		default: return 1;
		}

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

		switch (shaderType)
		{
		case EShaderType::VertexShader:
			{
				result = m_device->CreateVertexShader(m_vertexShader->getByteCode(), m_vertexShader->getByteCodeSize(),
				                                      nullptr, &m_vertexShader->m_shaderPtr);
				break;
			}
		case EShaderType::PixelShader:
			{
				result = m_device->CreatePixelShader(m_pixelShader->getByteCode(), m_pixelShader->getByteCodeSize(),
				                                     nullptr, &m_pixelShader->m_shaderPtr);
				break;
			}
		}
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed compiling shader ({}).", formatHResult(result));
			shader = nullptr;
			return result;
		}

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

	void setVertexData(float* data, size_t size, int32 count) override
	{
		m_vertexDataArray = (float*)PlatformMemory::malloc(size);
		std::memcpy(m_vertexDataArray, data, size);
		m_vertexDataSize = size;
		m_vertexCount    = count;

		D3D11_BUFFER_DESC vertexBufferDesc           = {};
		D3D11_SUBRESOURCE_DATA vertexSubResourceData = {nullptr};
		vertexBufferDesc.ByteWidth                   = m_vertexDataSize;
		vertexBufferDesc.Usage                       = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags                   = D3D11_BIND_VERTEX_BUFFER;
		vertexSubResourceData.pSysMem                = m_vertexDataArray;

		m_vertexBuffer = nullptr;
		HRESULT result = m_device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, &m_vertexBuffer);
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create vertex buffer ({}).", formatHResult(result));
		}
	}

	void setIndexData(int32* data, size_t size, int32 count) override
	{
		m_indexDataArray = (uint32*)PlatformMemory::malloc(size);
		std::memcpy(m_indexDataArray, data, size);
		m_indexDataSize = size;
		m_indexCount    = count;

		D3D11_BUFFER_DESC bufferDesc           = {};
		D3D11_SUBRESOURCE_DATA subResourceData = {nullptr};
		bufferDesc.ByteWidth                   = m_indexDataSize;
		bufferDesc.Usage                       = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags                   = D3D11_BIND_INDEX_BUFFER;
		subResourceData.pSysMem                = m_indexDataArray;

		m_indexBuffer  = nullptr;
		HRESULT result = m_device->CreateBuffer(&bufferDesc, &subResourceData, &m_indexBuffer);
		if (FAILED(result))
		{
			LOG_ERROR("D3D11RenderPipeline::init(): Failed to create index buffer ({}).", formatHResult(result));
		}
	}
};
