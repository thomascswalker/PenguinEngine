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

#include "RenderPipeline.h"

#include "Core/String.h"

#include "Renderer/Camera.h"
#include "Renderer/Shader.h"

using Microsoft::WRL::ComPtr;

#define CHECK_RESULT(func) \
	if (!(func))           \
	{                      \
		return false;      \
	}

#define CHECK_HR(result, msg)                                                                 \
	if (FAILED(result))                                                                       \
	{                                                                                         \
		LOG_ERROR("D3D11RenderPipeline::{}(): {} ({})", __func__, msg, formatHResult(result)) \
		return false;                                                                         \
	}

inline const char* formatHResult(const HRESULT err)
{
	auto comErr    = _com_error(err);
	LPCTSTR errMsg = comErr.ErrorMessage();
	CStringA errMsgA(errMsg);
	const char* errMsgP = errMsgA;
	return errMsgP;
}

// 268 bytes (aligned as 272)
struct ConstantData
{
	DirectX::XMMATRIX mvp;             // 64 bytes
	DirectX::XMMATRIX model;           // 64 bytes
	DirectX::XMMATRIX view;            // 64 bytes
	DirectX::XMMATRIX projection;      // 64 bytes
	DirectX::XMFLOAT3 cameraDirection; // 12 bytes
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
/** https://walbourn.github.io/anatomy-of-direct3d-11-create-device/ **/
/** https://antongerdelan.net/opengl/d3d11.html **/
class D3D11RenderPipeline : public IRenderPipeline
{
	bool m_initialized = false;
	HWND m_hwnd        = nullptr;

	int32 m_width  = 0;
	int32 m_height = 0;

	int32 m_sampleCount   = 1;
	int32 m_sampleQuality = 0;
	int32 m_mipLevels     = 1;

	RenderSettings m_renderSettings;

	/** Boilerplate D3D11 **/

	D3D_FEATURE_LEVEL m_featureLevel            = D3D_FEATURE_LEVEL_11_0;
	ComPtr<ID3D11Device> m_device               = nullptr;
	ComPtr<ID3D11DeviceContext> m_deviceContext = nullptr;
	ComPtr<IDXGISwapChain> m_swapChain          = nullptr;

	/** Buffers **/

	ComPtr<ID3D11RenderTargetView> m_renderTargetView = nullptr;

	ComPtr<ID3D11DepthStencilView> m_depthStencilView = nullptr;
	ComPtr<ID3D11Texture2D> m_depthStencilTexture     = nullptr;
	ComPtr<ID3D11Texture2D> m_backBuffer              = nullptr;

	/** Vertex & Index Buffer **/

	ComPtr<ID3D11Buffer> m_vertexBuffer = nullptr;
	float* m_vertexDataArray            = nullptr;
	size_t m_vertexDataSize             = 0;
	uint32 m_vertexStride               = 6 * sizeof(float); // (Pos.XYZ + Norm.XYZ) * 4 = 24
	uint32 m_vertexOffset               = 0;
	uint32 m_vertexCount                = 0;

	ComPtr<ID3D11Buffer> m_indexBuffer = nullptr;
	uint32* m_indexDataArray           = nullptr;
	size_t m_indexDataSize             = 0;
	uint32 m_indexStride               = 3 * sizeof(int32);
	uint32 m_indexOffset               = 0;
	uint32 m_indexCount                = 0;

	/** Shaders **/

	std::map<const char*, Shader*> m_shaders;
	ComPtr<ID3D11InputLayout> m_vertexInputLayout = nullptr;
	D3D11VertexShader* m_vertexShader             = nullptr;
	D3D11PixelShader* m_pixelShader               = nullptr;

	/** Camera **/

	ComPtr<ID3D11Buffer> m_constantDataBuffer = nullptr;

public:
	~D3D11RenderPipeline() override = default;

	bool createDevice();
	bool createSwapChain();
	bool createBackBuffer();
	bool createRenderTargetView();
	bool createShaders();
	bool createDepthBuffer();
	bool createConstantBuffer();
	[[nodiscard]] bool createRasterizerState() const;
	[[nodiscard]] bool createViewport() const;

	bool init(void* windowHandle) override;
	void beginDraw() override;
	void draw() override;
	void endDraw() override;
	void shutdown() override;
	void resize(int32 width, int32 height) override;
	void drawGrid(Grid* grid) override;
	void drawLine(const vec3f& inA, const vec3f& inB, const Color& color) override;
	uint8* getFrameData() override;
	void setViewData(ViewData* newViewData) override;
	void setRenderSettings(RenderSettings* newRenderSettings) override;
	void setHwnd(HWND hwnd);

	HRESULT createShader(const char* name, const std::string& fileName, EShaderType shaderType);
	static HRESULT compileShader(LPCWSTR fileName, LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob);
	void setVertexData(float* data, size_t size, int32 count) override;
};
