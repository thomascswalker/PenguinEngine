#include "D3D11.h"

#include <d3dcompiler.h>
#include <directxcolors.h>
#include <filesystem>

bool D3D11RenderPipeline::createDevice()
{
	DWORD createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr, 0,
	                                   D3D11_SDK_VERSION, m_device.GetAddressOf(), &m_featureLevel,
	                                   m_deviceContext.GetAddressOf());
	if (FAILED(result))
	{
		LOG_ERROR("D3D11RenderPipeline::init(): Failed to create create device.")
		return false;
	}

	return true;
}

bool D3D11RenderPipeline::createSwapChain()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc               = {};
	swapChainDesc.BufferCount                        = 1; // 1 Front and back buffer
	swapChainDesc.BufferDesc.Width                   = g_defaultViewportWidth;
	swapChainDesc.BufferDesc.Height                  = g_defaultViewportHeight;
	swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow                       = m_hwnd;
	swapChainDesc.SampleDesc.Count                   = 1;
	swapChainDesc.SampleDesc.Quality                 = 0;
	swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed                           = TRUE;

	HRESULT result = m_dxgiFactory->CreateSwapChain(m_device.Get(), &swapChainDesc, m_swapChain.GetAddressOf());
	if (FAILED(result))
	{
		LOG_ERROR("D3D11RenderPipeline::init(): Failed to create swap chain ({}).", formatHResult(result))
		return false;
	}

	return true;
}

bool D3D11RenderPipeline::createDxgiDevice()
{
	// Obtain DXGI Factory from the device, since we used nullptr for the pAdapter argument of D3D11CreateDevice
	HRESULT result = m_device.As(&m_dxgiDevice);
	if (SUCCEEDED(result))
	{
		IDXGIAdapter* adapter = nullptr;
		result                = m_dxgiDevice->GetAdapter(&adapter);
		if (SUCCEEDED(result))
		{
			result = adapter->GetParent(IID_PPV_ARGS(&m_dxgiFactory));
			if (FAILED(result))
			{
				LOG_ERROR("D3D11RenderPipeline::init(): Failed to get adapter parent.")
				return false;
			}
			adapter->Release();
		}
		m_dxgiDevice->Release();
	}
	else
	{
		LOG_ERROR("D3D11RenderPipeline::init(): Failed to obtain DXGI Factory.")
		return false;
	}
	return true;
}

bool D3D11RenderPipeline::makeWindowAssociation()
{
	HRESULT result = m_dxgiFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
	if (FAILED(result))
	{
		LOG_ERROR("D3D11RenderPipeline::init(): Failed to create associate HWND {} with DXGI Factory.",
		          (int32)m_hwnd)
		return false;
	}
	m_dxgiFactory->Release();
	return true;
}

bool D3D11RenderPipeline::createFrameBuffer()
{
	HRESULT result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
	                                        reinterpret_cast<void**>(m_frameBuffer.GetAddressOf()));
	if (FAILED(result))
	{
		LOG_ERROR("D3D11RenderPipeline::init(): Failed to create backbuffer ({}).", formatHResult(result))
		return false;
	}
	return true;
}

bool D3D11RenderPipeline::createRenderTargetView()
{
	HRESULT result = m_device->CreateRenderTargetView(m_frameBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
	if (FAILED(result))
	{
		LOG_ERROR("D3D11RenderPipeline::init(): Failed to create render target view ({}).", formatHResult(result))
		return false;
	}
	return true;
}

bool D3D11RenderPipeline::createShaders()
{
	HRESULT result = createShader("VTX", "VertexShader.hlsl", EShaderType::VertexShader);
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
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
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
	return true;
}

bool D3D11RenderPipeline::createDepthBuffer()
{
	// Depth buffer
	D3D11_TEXTURE2D_DESC depthTextureDesc;
	ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
	depthTextureDesc.Width            = g_defaultViewportWidth;
	depthTextureDesc.Height           = g_defaultViewportHeight;
	depthTextureDesc.MipLevels        = 1;
	depthTextureDesc.ArraySize        = 1;
	depthTextureDesc.SampleDesc.Count = 1;
	depthTextureDesc.Format           = DXGI_FORMAT_D32_FLOAT;
	depthTextureDesc.BindFlags        = D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.Usage            = D3D11_USAGE_DEFAULT;
	depthTextureDesc.CPUAccessFlags   = 0;
	depthTextureDesc.MiscFlags        = 0;

	HRESULT result = m_device->CreateTexture2D(&depthTextureDesc, nullptr, &m_depthStencilTexture);
	if (FAILED(result))
	{
		LOG_ERROR("D3D11RenderPipeline::init(): Failed creating depth stencil texture ({}).",
		          formatHResult(result));
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));

	// Depth test parameters
	dsDesc.DepthEnable      = true;
	dsDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc        = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable    = false;
	dsDesc.StencilReadMask  = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	result = m_device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		LOG_ERROR("D3D11RenderPipeline::init(): Failed creating depth stencil state ({}).", formatHResult(result));
		return false;
	}
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Format             = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.Flags              = 0;
	depthStencilViewDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view
	result = m_device->CreateDepthStencilView(m_depthStencilTexture.Get(), // Depth stencil texture
	                                          &depthStencilViewDesc,       // Depth stencil desc
	                                          &m_depthBuffer);             // [out] Depth stencil view
	if (FAILED(result))
	{
		LOG_ERROR("D3D11RenderPipeline::init(): Failed creating depth stencil view ({}).",
		          formatHResult(result));
		return false;
	}

	return true;
}

bool D3D11RenderPipeline::createConstantBuffer()
{
	// Constant Buffer to pass camera properties to the shaders
	// https://samulinatri.com/blog/direct3d-11-constant-buffer-tutorial
	D3D11_BUFFER_DESC constantDataBufferDesc;
	constantDataBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	constantDataBufferDesc.ByteWidth           = 272; // see ConstantBuffer, size 204 but needs to be multiple of 16
	constantDataBufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	constantDataBufferDesc.CPUAccessFlags      = 0;
	constantDataBufferDesc.MiscFlags           = 0;
	constantDataBufferDesc.StructureByteStride = 0;

	HRESULT result = m_device->CreateBuffer(&constantDataBufferDesc, nullptr, m_constantDataBuffer.GetAddressOf());
	if (FAILED(result))
	{
		LOG_ERROR("D3D11RenderPipeline::init(): Failed creating the camera constant buffer ({}).",
		          formatHResult(result));
		return false;
	}
	m_deviceContext->VSSetConstantBuffers(0, 1, m_constantDataBuffer.GetAddressOf());
	return true;
}

bool D3D11RenderPipeline::init(void* windowHandle)
{
	m_hwnd = (HWND)windowHandle;
	ASSERT(m_hwnd != nullptr, "D3D11RenderPipeline::init(): HWND not set.");

	CHECK_RESULT(createDevice())
	CHECK_RESULT(createDxgiDevice())
	CHECK_RESULT(createSwapChain())
	CHECK_RESULT(makeWindowAssociation())
	CHECK_RESULT(createFrameBuffer())
	CHECK_RESULT(createRenderTargetView())
	CHECK_RESULT(createShaders())
	CHECK_RESULT(createDepthBuffer())
	CHECK_RESULT(createConstantBuffer())

	m_initialized = true;

	return true;
}

void D3D11RenderPipeline::beginDraw()
{
	if (!m_initialized)
	{
		LOG_ERROR("D3D11RenderPipeline::beginDraw(): Pipeline is not initialized.")
		assert(false);
	}
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), DirectX::Colors::Black);

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
	rasterDesc.CullMode              = D3D11_CULL_BACK;
	rasterDesc.FillMode              = D3D11_FILL_SOLID;
	rasterDesc.DepthClipEnable       = false;
	rasterDesc.DepthBias             = 0;
	rasterDesc.DepthBiasClamp        = 0.0f;
	rasterDesc.FrontCounterClockwise = true;

	HRESULT result = m_device->CreateRasterizerState(&rasterDesc, &rasterState);
	if (FAILED(result))
	{
		LOG_ERROR("D3D11RenderPipeline::beginDraw(): Failed creating rasterizer state ({}).",
		          formatHResult(result));
		assert(false);
	}
	m_deviceContext->RSSetState(rasterState);

	// At the end, set the render targets to the main framebuffer and the depthbuffer
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr); // m_depthBuffer

	// Associate the vertex and index buffers with the device context
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->IASetInputLayout(m_inputLayout.Get());
	m_deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_vertexStride, &m_vertexOffset);

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

void D3D11RenderPipeline::draw()
{
	m_deviceContext->Draw(m_vertexCount, 0);
	HRESULT result = m_swapChain->Present(1, 0);
	if (FAILED(result))
	{
		LOG_ERROR("D3D11RenderPipeline::init(): Failed to present swap chain.")
		assert(false);
	}
}

void D3D11RenderPipeline::endDraw()
{
	// Clear the depth buffer
	if (m_depthBuffer != nullptr)
	{
		m_deviceContext->ClearDepthStencilView(m_depthBuffer.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
}

void D3D11RenderPipeline::shutdown()
{
	m_deviceContext->ClearState();

	m_swapChain->Release();
	m_deviceContext->Release();
	m_device->Release();
	m_frameBuffer->Release();
	m_inputLayout->Release();

	m_initialized = false;
}

void D3D11RenderPipeline::resize(int32 width, int32 height)
{
	m_width  = width;
	m_height = height;
}

void D3D11RenderPipeline::drawGrid(Grid* grid) {}
void D3D11RenderPipeline::drawLine(const vec3f& inA, const vec3f& inB, const Color& color) {}

uint8* D3D11RenderPipeline::getFrameData()
{
	return nullptr;
}

void D3D11RenderPipeline::setViewData(ViewData* newViewData)
{
	ConstantData data;
	std::memcpy(data.mvp, newViewData->viewProjectionMatrix.m, sizeof(mat4f));
	std::memcpy(data.model, newViewData->modelMatrix.m, sizeof(mat4f));
	std::memcpy(data.view, newViewData->viewMatrix.m, sizeof(mat4f));
	std::memcpy(data.projection, newViewData->projectionMatrix.m, sizeof(mat4f));
	std::memcpy(data.cameraDirection, newViewData->cameraDirection.xyz, sizeof(vec3f));

	m_deviceContext->UpdateSubresource(m_constantDataBuffer.Get(), 0, nullptr, &data, 0, 0);
}

void D3D11RenderPipeline::setRenderSettings(RenderSettings* newRenderSettings) {}

void D3D11RenderPipeline::setHwnd(const HWND hwnd)
{
	m_hwnd = hwnd;
}

HRESULT D3D11RenderPipeline::createShader(const char* name, const std::string& fileName, EShaderType shaderType)
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

HRESULT D3D11RenderPipeline::compileShader(const LPCWSTR fileName, const LPCSTR entryPoint, const LPCSTR profile,
                                           ID3DBlob** blob)
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

void D3D11RenderPipeline::setVertexData(float* data, size_t size, int32 count)
{
	m_vertexDataArray = (float*)PlatformMemory::malloc(size);
	std::memcpy(m_vertexDataArray, data, size);
	m_vertexDataSize = size;
	m_vertexCount    = count;

	D3D11_BUFFER_DESC vertexBufferDesc           = {};
	D3D11_SUBRESOURCE_DATA vertexSubResourceData = {nullptr};
	vertexBufferDesc.ByteWidth                   = m_vertexDataSize;
	vertexBufferDesc.Usage                       = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags                   = D3D11_BIND_VERTEX_BUFFER;
	vertexSubResourceData.pSysMem                = m_vertexDataArray;

	m_vertexBuffer = nullptr;
	HRESULT result = m_device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, m_vertexBuffer.GetAddressOf());
	if (FAILED(result))
	{
		LOG_ERROR("D3D11RenderPipeline::init(): Failed to create vertex buffer ({}).", formatHResult(result));
	}
}
