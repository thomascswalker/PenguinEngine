// ReSharper disable CppClangTidyClangDiagnosticLanguageExtensionToken
#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11.lib")		// direct3D library
#pragma comment(lib, "dxgi.lib")		// directx graphics interface
#pragma comment(lib, "d3dcompiler.lib") // shader compiler

#include <atlstr.h>
#include <comdef.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <dxgi.h>
#include <map>
#include <Windows.h>

#include <wrl/client.h>

#include "Engine/Buffer.h"
#include "RHI.h"
#include "Core/String.h"
#include "Engine/Actors/Camera.h"
#include "Renderer/Shader.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

// Global reference to D3D11 Device
inline ID3D11Device* g_device = nullptr;
// Global reference to D3D11 Device Context
inline ID3D11DeviceContext* g_deviceContext = nullptr;

#define CHECK_RESULT(func) \
	if (!(func))           \
	{                      \
		return false;      \
	}

#define CHECK_HR(result, msg)                                                      \
	if (FAILED(result))                                                            \
	{                                                                              \
		LOG_ERROR("D3D11RHI::{}(): {} ({})", __func__, msg, formatHResult(result)) \
		return false;                                                              \
	}

inline const char* formatHResult(const HRESULT err)
{
	auto		comErr = _com_error(err);
	LPCTSTR		errMsg = comErr.ErrorMessage();
	CStringA	errMsgA(errMsg);
	const char* errMsgP = errMsgA;
	return errMsgP;
}

/** Constant Buffers **/

constexpr uint32 g_constantBufferCount = 1;

namespace ConstantBufferId
{
	constexpr uint32 Camera = 0;
	constexpr uint32 Model = 1;
}; // namespace ConstantBufferId

struct CBCamera
{
	XMMATRIX viewProjection;
	XMFLOAT4 cameraDirection;
};

struct CBModel
{
	XMMATRIX model;
};

/** Shaders **/

class VertexShader11 : public VertexShader
{
	ID3D11VertexShader* m_ptr = nullptr;

public:
	~VertexShader11() override
	{
		if (m_ptr)
		{
			m_ptr->Release();
		}
	}

	ID3D11VertexShader* getDXShader() const
	{
		return m_ptr;
	}

	ID3D11VertexShader* const* getDXShaderPtr() const throw()
	{
		return &m_ptr;
	}

	ID3D11VertexShader** getDXShaderPtr() throw()
	{
		return &m_ptr;
	}
};

class PixelShader11 : public PixelShader
{
	ID3D11PixelShader*				 m_ptr = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_shaderResourceView = nullptr;
	ComPtr<ID3D11SamplerState>		 m_samplerState = nullptr;

public:
	~PixelShader11() override
	{
		if (m_ptr)
		{
			m_ptr->Release();
		}
	}

	ID3D11PixelShader* getDXShader() const
	{
		return m_ptr;
	}

	ID3D11PixelShader* const* getDXShaderPtr() const throw()
	{
		return &m_ptr;
	}

	ID3D11PixelShader** getDXShaderPtr() throw()
	{
		return &m_ptr;
	}

	void setShaderResourceView(ID3D11ShaderResourceView* view)
	{
		m_shaderResourceView = view;
	}

	ID3D11ShaderResourceView* getShaderResourceView() const
	{
		return m_shaderResourceView.Get();
	}

	void setSamplerState(ID3D11SamplerState* state)
	{
		m_samplerState = state;
	}

	ID3D11SamplerState* getSamplerState() const
	{
		return m_samplerState.Get();
	}
};

/** Buffers **/

class Buffer11 : public GenericBuffer
{
	MeshDescription		 m_meshDescription;
	ComPtr<ID3D11Buffer> m_vertexBuffer = nullptr;
	ComPtr<ID3D11Buffer> m_constantBuffer = nullptr;

public:
	void createVertexBuffer(std::vector<float>& data) override;
	void createConstantBuffer(int32 byteSize) override;
	void setMeshDescription(const MeshDescription& meshDescription) { m_meshDescription = meshDescription; }

	MeshDescription* getMeshDescription() { return &m_meshDescription; }
	ID3D11Buffer*	 getVertexBuffer() const { return m_vertexBuffer.Get(); }
	ID3D11Buffer*	 getConstantBuffer() const { return m_constantBuffer.Get(); }
};

// https://gist.github.com/d7samurai/261c69490cce0620d0bfc93003cd1052
/** https://walbourn.github.io/anatomy-of-direct3d-11-create-device/ **/
/** https://antongerdelan.net/opengl/d3d11.html **/
class D3D11RHI : public IRHI
{
	bool m_initialized = false;
	HWND m_hwnd = nullptr;

	int32 m_width = g_defaultViewportWidth;
	int32 m_height = g_defaultViewportHeight;

	int32 m_sampleCount = 1;
	int32 m_sampleQuality = 0;
	int32 m_mipLevels = 1;

	RenderSettings m_renderSettings;

	/** Boilerplate D3D11 **/

	D3D_FEATURE_LEVEL			m_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ComPtr<ID3D11Device>		m_device = nullptr;
	ComPtr<ID3D11DeviceContext> m_deviceContext = nullptr;
	ComPtr<IDXGISwapChain>		m_swapChain = nullptr;

	/** Buffers **/

	ComPtr<ID3D11RenderTargetView> m_renderTargetView = nullptr;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView = nullptr;
	ComPtr<ID3D11Texture2D>		   m_depthStencilTexture = nullptr;
	ComPtr<ID3D11Texture2D>		   m_backBuffer = nullptr;

	/** Vertex & Index Buffer **/

	ID3D11Buffer*					 m_constantBuffers[g_constantBufferCount];
	std::vector<Buffer11>			 m_meshBuffers;

	/** Shaders **/

	std::map<const char*, Shader*> m_shaders;
	ID3D11InputLayout*			   m_vertexInputLayout = nullptr;
	VertexShader11*				   m_vertexShader = nullptr;
	PixelShader11*				   m_pixelShader = nullptr;

	/** Camera **/

	ViewData* m_viewData = nullptr;

public:
	~D3D11RHI() override = default;

	bool createDevice();
	bool createSwapChain();
	bool createBackBuffer();
	bool createRenderTargetView();
	bool createShaders();
	void  createDefaultShader();
	bool createDepthBuffer();
	bool createInputLayout();
	bool createConstantBuffers();

	[[nodiscard]] bool createRasterizerState() const;
	[[nodiscard]] bool createViewport() const;

	bool   init(void* windowHandle) override;
	void   beginDraw() override;
	void   draw() override;
	void   drawMesh(Buffer11* buffer);
	void   endDraw() override;
	void   shutdown() override;
	void   resize(int32 width, int32 height) override;
	void   drawGrid(Grid* grid) override;
	void   drawLine(const vec3f& inA, const vec3f& inB, const Color& color) override;
	uint8* getFrameData() override;
	void   setViewData(ViewData* newViewData) override;
	void   setRenderSettings(RenderSettings* newRenderSettings) override;
	void   setHwnd(HWND hwnd);

	HRESULT		   createShader(const char* name, const std::string& fileName, EShaderType shaderType);
	static HRESULT compileShader(LPCWSTR fileName, LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob);

	void addRenderable(IRenderable* renderable) override;
	void addTexture(Texture* texture) override;
};
