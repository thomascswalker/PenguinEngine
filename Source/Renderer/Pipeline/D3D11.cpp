#pragma warning(disable : 6385)

#include "D3D11.h"

#include <d3dcompiler.h>
#include <directxcolors.h>
#include <filesystem>

#include "D3D11Core.h"

using namespace DirectX;

bool D3D11RHI::createDevice()
{
	LOG_DEBUG("Creating D3D11 Device.")
	DWORD createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr, 0,
		D3D11_SDK_VERSION, m_device.GetAddressOf(), &m_featureLevel,
		m_deviceContext.GetAddressOf());
	CHECK_HR(result, "Failed to create create device.")

	g_device = m_device.Get();
	g_deviceContext = m_deviceContext.Get();

	return true;
}

bool D3D11RHI::createSwapChain()
{
	LOG_DEBUG("Creating D3D11 Swap Chain.")
	IDXGIDevice*  dxgiDevice = nullptr;
	IDXGIFactory* dxgiFactory = nullptr;
	IDXGIAdapter* dxgiAdapter = nullptr;

	HRESULT result = m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	CHECK_HR(result, "Failed to obtain DXGI device");

	result = dxgiDevice->GetAdapter(&dxgiAdapter);
	CHECK_HR(result, "Failed to obtain DXGI adapter");

	result = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
	CHECK_HR(result, "Failed to get adapter parent");

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 1; // 1 Front and back buffer
	swapChainDesc.BufferDesc.Width = g_defaultViewportWidth;
	swapChainDesc.BufferDesc.Height = g_defaultViewportHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = m_hwnd;
	swapChainDesc.SampleDesc = DXGI_SAMPLE_DESC(m_sampleCount, m_sampleQuality);
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed = TRUE;

	result = dxgiFactory->CreateSwapChain(m_device.Get(), &swapChainDesc, m_swapChain.GetAddressOf());
	CHECK_HR(result, "Failed to create swap chain");

	dxgiAdapter->Release();
	dxgiDevice->Release();
	dxgiFactory->Release();

	CHECK_RESULT(createBackBuffer())

	return true;
}

bool D3D11RHI::createBackBuffer()
{
	LOG_DEBUG("Creating D3D11 Frame Buffer.")
	HRESULT result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(m_backBuffer.GetAddressOf()));
	CHECK_HR(result, "Failed to create backbuffer");
	return true;
}

bool D3D11RHI::createRenderTargetView()
{
	LOG_DEBUG("Creating D3D11 Render Target View.")
	HRESULT result = m_device->CreateRenderTargetView(m_backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
	CHECK_HR(result, "Failed to create render target view");
	return true;
}

bool D3D11RHI::createShaders()
{
	LOG_DEBUG("Creating D3D11 Vertex and Pixel Shaders.")
	HRESULT result = createShader("VTX", "VertexShader.hlsl", EShaderType::VertexShader);
	CHECK_HR(result, "Failed to create vertex shader");
	result = createShader("PXL", "PixelShader.hlsl", EShaderType::PixelShader);
	CHECK_HR(result, "Failed to create pixel shader");

	// Set the vertex and pixel shaders on the device context
	if (m_vertexShader->m_shaderPtr)
	{
		m_deviceContext->VSSetShader(m_vertexShader->m_shaderPtr, nullptr, 0);
	}
	if (m_pixelShader->m_shaderPtr)
	{
		m_deviceContext->PSSetShader(m_pixelShader->m_shaderPtr, nullptr, 0);
	}

	return true;
}

bool D3D11RHI::createDepthBuffer()
{
	LOG_DEBUG("Creating D3D11 Depth Buffer.")

	// Depth buffer
	D3D11_TEXTURE2D_DESC depthTextureDesc;
	depthTextureDesc.Width = g_defaultViewportWidth;
	depthTextureDesc.Height = g_defaultViewportHeight;
	depthTextureDesc.MipLevels = m_mipLevels;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.SampleDesc = DXGI_SAMPLE_DESC(m_sampleCount, m_sampleQuality);
	depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTextureDesc.CPUAccessFlags = 0;
	depthTextureDesc.MiscFlags = 0;

	HRESULT result = m_device->CreateTexture2D(&depthTextureDesc, nullptr, m_depthStencilTexture.GetAddressOf());
	CHECK_HR(result, "Failed creating depth stencil texture");

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_device->CreateDepthStencilView(m_depthStencilTexture.Get(), // Depth stencil texture
		&depthStencilViewDesc,											   // Depth stencil desc ,
		m_depthStencilView.GetAddressOf());								   // [out] Depth stencil view
	CHECK_HR(result, "Failed creating depth stencil view");
	return true;
}

bool D3D11RHI::createInputLayout()
{
	// Create input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[3]{};

	inputElementDesc[0].SemanticName = "SV_POSITION";
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	inputElementDesc[1].SemanticName = "NORMAL";
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[1].AlignedByteOffset = 3 * sizeof(float);
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	inputElementDesc[2].SemanticName = "TEXCOORD";
	inputElementDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDesc[2].AlignedByteOffset = 6 * sizeof(float);
	inputElementDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	HRESULT result = g_device->CreateInputLayout(
		inputElementDesc,
		ARRAYSIZE(inputElementDesc),
		m_vertexShader->getByteCode(),
		m_vertexShader->getByteCodeSize(),
		&m_vertexInputLayout);
	CHECK_HR(result, "Failed creating vertex input layout.");
	return true;
}

// https://samulinatri.com/blog/direct3d-11-constant-buffer-tutorial
bool D3D11RHI::createConstantBuffers()
{
	LOG_DEBUG("Creating D3D11 Constant Buffers.")

	// Store buffer sizes in an array
	uint32 bufferSizes[g_constantBufferCount];
	bufferSizes[ConstantBufferId::Camera] = sizeof(CBCamera);
	bufferSizes[ConstantBufferId::Model] = sizeof(CBModel);

	// Create each buffer
	for (int32 i = 0; i < g_constantBufferCount; i++)
	{
		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = bufferSizes[i];
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		HRESULT result = m_device->CreateBuffer(&desc, nullptr, &m_constantBuffers[i]);
		CHECK_HR(result, "Failed creating the camera constant buffer.");
	}
	return true;
}

bool D3D11RHI::init(void* windowHandle)
{
	LOG_DEBUG("Initializing D3D11 pipeline.")

	m_hwnd = (HWND)windowHandle;
	// ASSERT(m_hwnd != nullptr, "D3D11RHI::init(): HWND not set.");

	CHECK_RESULT(createDevice())
	CHECK_RESULT(createSwapChain())
	CHECK_RESULT(createRenderTargetView())
	CHECK_RESULT(createShaders())
	CHECK_RESULT(createDepthBuffer())
	CHECK_RESULT(createInputLayout())
	CHECK_RESULT(createConstantBuffers())
	CHECK_RESULT(createViewport())
	CHECK_RESULT(createRasterizerState())

	// At the end, set the render targets to the main frame buffer and the depth bufferd
	LOG_DEBUG("Setting Render Targets.")
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

	m_initialized = true;

	return true;
}

bool D3D11RHI::createRasterizerState() const
{
	LOG_DEBUG("Creating D3D11 Raster State.")

	ID3D11RasterizerState* rasterState;
	D3D11_RASTERIZER_DESC  rasterDesc{};
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.MultisampleEnable = false;

	HRESULT result = m_device->CreateRasterizerState(&rasterDesc, &rasterState);
	CHECK_HR(result, "Failed creating rasterizer state");
	m_deviceContext->RSSetState(rasterState);

	return true;
}

bool D3D11RHI::createViewport() const
{
	LOG_DEBUG("Creating D3D11 Viewport.")
	RECT winRect;
	GetClientRect(m_hwnd, &winRect);
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (FLOAT)(winRect.right - winRect.left);
	viewport.Height = (FLOAT)(winRect.bottom - winRect.top);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_deviceContext->RSSetViewports(1, &viewport);

	return true;
}

void D3D11RHI::beginDraw()
{
	if (!m_initialized)
	{
		auto msg = "D3D11RHI::beginDraw(): Pipeline is not initialized.";
		ASSERT(false, msg);
		return;
	}
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::Black);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Associate the vertex and index buffers with the device context
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->IASetInputLayout(m_vertexInputLayout);
}

void D3D11RHI::draw()
{
	for (auto& buffer : m_meshBuffers)
	{
		drawMesh(&buffer);
	}

	HRESULT result = m_swapChain->Present(1, 0);
	if (FAILED(result))
	{
		auto msg = "D3D11RHI::init(): Failed to present swap chain.";
		ASSERT(false, msg);
	}
}

void D3D11RHI::drawMesh(Buffer11* buffer)
{
	// Buffers
	ID3D11Buffer* vertexBufferData = buffer->getVertexBuffer();
	ID3D11Buffer* constantBufferData = buffer->getConstantBuffer();

	// Update model constant buffer
	mat4f	model = buffer->getMeshDescription()->transform->toMatrix();
	CBModel data{};
	data.model = XMMATRIX(&model.m[0][0]);
	m_deviceContext->UpdateSubresource(constantBufferData, 0, nullptr, &data, 0, 0);

	// Get mesh info
	auto   desc = buffer->getMeshDescription();
	uint32 vertexBufferStride = desc->stride;
	uint32 vertexBufferOffset = 0;

	// Set the current vertex buffer to this mesh's buffer
	m_deviceContext->IASetVertexBuffers(0, 1, &vertexBufferData, &vertexBufferStride, &vertexBufferOffset);

	// Set the constant buffer for this mesh
	m_deviceContext->VSSetConstantBuffers(0 /* Camera */, 1, &m_constantBuffers[ConstantBufferId::Camera]);
	m_deviceContext->VSSetConstantBuffers(1 /* Model */, 1, &constantBufferData);

	// Draw the mesh to the screen
	m_deviceContext->Draw(desc->byteSize, 0);
}

void D3D11RHI::addRenderable(IRenderable* renderable)
{
	Buffer11 buffer;
	auto	 vertexData = renderable->getMesh()->getVertexData();
	buffer.createVertexBuffer(*vertexData);
	buffer.createConstantBuffer();

	MeshDescription meshDesc;
	meshDesc.stride = sizeof(Vertex);
	meshDesc.data = vertexData->data();
	meshDesc.byteSize = vertexData->size();
	meshDesc.transform = renderable->getTransform();
	buffer.setMeshDescription(meshDesc);

	m_meshBuffers.emplace_back(buffer);
}

void D3D11RHI::endDraw()
{
	m_deviceContext->OMSetDepthStencilState(nullptr, 0);
}

void D3D11RHI::shutdown()
{
	m_deviceContext->ClearState();

	m_initialized = false;
}

void D3D11RHI::resize(int32 width, int32 height)
{
	m_width = width;
	m_height = height;
}

void D3D11RHI::drawGrid(Grid* grid) {}
void D3D11RHI::drawLine(const vec3f& inA, const vec3f& inB, const Color& color) {}

uint8* D3D11RHI::getFrameData()
{
	return nullptr;
}

void D3D11RHI::setViewData(ViewData* newViewData)
{
	// Store new view data
	m_viewData = newViewData;

	// Convert from native to DX types
	XMMATRIX viewProjection = XMMATRIX(&m_viewData->viewProjectionMatrix.m[0][0]);

	// Create constant buffer
	CBCamera data{};
	data.viewProjection = XMMatrixTranspose(viewProjection);
	data.cameraDirection = XMFLOAT4(m_viewData->cameraDirection.x, m_viewData->cameraDirection.y, m_viewData->cameraDirection.z, 0);

	// Update the buffer data
	ID3D11Buffer* cameraBuffer = m_constantBuffers[ConstantBufferId::Camera];
	m_deviceContext->UpdateSubresource(cameraBuffer, 0, nullptr, &data, 0, 0);
}

void D3D11RHI::setRenderSettings(RenderSettings* newRenderSettings)
{
	m_renderSettings = *newRenderSettings;
}

void D3D11RHI::setHwnd(const HWND hwnd)
{
	m_hwnd = hwnd;
}

HRESULT D3D11RHI::createShader(const char* name, const std::string& fileName, EShaderType shaderType)
{
	Shader*		shader;
	std::string profile;
	switch (shaderType)
	{
		case EShaderType::VertexShader:
		{
			m_vertexShader = new VertexShader11();
			shader = m_vertexShader;
			profile = "vs_5_0";
			break;
		}
		case EShaderType::PixelShader:
		{
			m_pixelShader = new PixelShader11();
			shader = m_pixelShader;
			profile = "ps_5_0";
			break;
		}
		default:
			return 1;
	}

	// TODO: Currently D3DCompileFromFile cannot find relative files, so we have to build the path from the current file.

	auto currentFile = std::filesystem::path(__FILE__);
	auto currentPath = currentFile.remove_filename();
	auto shaderFileName = currentPath;
	shaderFileName.append(fileName);
	shader->setFileName(Strings::toString(shaderFileName.c_str()));

	ID3DBlob* blob = nullptr;
	HRESULT	  result = compileShader(shaderFileName.c_str(), "main", profile.c_str(), &blob);
	if (FAILED(result))
	{
		if (blob)
		{
			blob->Release();
		}
		LOG_ERROR("D3D11RHI::init(): Failed compiling vertex shader ({}).", formatHResult(result));
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
		LOG_ERROR("D3D11RHI::init(): Failed compiling shader ({}).", formatHResult(result));
		shader = nullptr;
		return result;
	}

	return S_OK;
}

HRESULT D3D11RHI::compileShader(const LPCWSTR fileName, const LPCSTR entryPoint, const LPCSTR profile,
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
	ID3DBlob* errorBlob = nullptr;
	HRESULT	  result = D3DCompileFromFile(fileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		  entryPoint, profile,
		  flags, 0, &shaderBlob, &errorBlob);
	if (FAILED(result))
	{
		if (errorBlob)
		{
			const char* errorMessage = (char*)errorBlob->GetBufferPointer();
			LOG_ERROR("D3D11RHI::compileShader(): Failed to compile shader ({}).", errorMessage)
			errorBlob->Release();
		}
		else
		{
			LOG_ERROR("D3D11RHI::compileShader(): Shader file not found ({}).",
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

inline void Buffer11::createVertexBuffer(std::vector<float>& vertexData)
{
	auto msg = "Buffer11::createVertexBuffer(): ID3D11Device is not instantiated.";
	ASSERT(g_device != nullptr, msg);

	D3D11_BUFFER_DESC	   bufferDesc{};
	D3D11_SUBRESOURCE_DATA subResourceData{};
	bufferDesc.ByteWidth = vertexData.size() * sizeof(float);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	subResourceData.pSysMem = vertexData.data();

	HRESULT result = g_device->CreateBuffer(&bufferDesc, &subResourceData, m_vertexBuffer.GetAddressOf());
	if (FAILED(result))
	{
		LOG_ERROR("D3D11Buffer::createVertexBuffer(): Failed to create vertex buffer ({}).", formatHResult(result));
	}
}

void Buffer11::createConstantBuffer()
{
	auto msg = "Buffer11::createVertexBuffer(): ID3D11Device is not instantiated.";
	ASSERT(g_device != nullptr, msg);

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(CBModel);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	HRESULT result = g_device->CreateBuffer(&bufferDesc, nullptr, m_constantBuffer.GetAddressOf());
	if (FAILED(result))
	{
		LOG_ERROR("D3D11Buffer::createConstantBuffer(): Failed to create constant buffer ({}).", formatHResult(result));
	}
}
