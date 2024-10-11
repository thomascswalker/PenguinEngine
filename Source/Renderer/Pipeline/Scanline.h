#pragma once

#include <memory>

#include "RenderPipeline.h"

#include "Engine/Actors/Camera.h"
#include "Engine/Mesh.h"
#include "Renderer/Grid.h"
#include "Renderer/Settings.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"

struct VertexInput
{
	vec3f position;
	vec3f normal;
	mat4f viewProjection;
	mat4f model;
};

struct VertexOutput
{
	vec4f position;
	vec3f normal;
};

struct PixelData
{
	int32 width;
	int32 height;
	vec2f position;
	vec3f normal;
	vec2f uv;
	float depth;
	vec3f worldPosition;
	vec3f worldNormal;
	vec3f cameraNormal;
	vec3f distance;
	Texture* texture;
};

class ScanlineVertexShader : public VertexShader
{
public:
	ScanlineVertexShader() = default;

	static VertexOutput process(const VertexInput& input);
};

class ScanlinePixelShader : public PixelShader
{
public:
	ScanlinePixelShader() = default;

	static Color process(const PixelData& input);
};

class ScanlineRenderPipeline : public IRenderPipeline
{
	std::shared_ptr<ScanlineVertexShader> m_vertexShader = nullptr;
	std::shared_ptr<ScanlinePixelShader> m_pixelShader   = nullptr;

	std::shared_ptr<Texture> m_frameBuffer = nullptr;
	std::shared_ptr<Texture> m_depthBuffer = nullptr;

	std::shared_ptr<ViewData> m_viewData = nullptr;

	/** Current model matrix **/
	mat4f m_modelMatrix;
	/** Vector of all vertexes in all meshes. **/
	std::vector<float> m_vertexBuffer;
	/** Vector of mesh descriptions of meshes which are currently bound. **/
	std::vector<MeshDescription> m_meshDescriptions;
	/** Pointer to the first Vertex in the current triangle. **/
	Vertex* m_vertexBufferPtr;
	/** Pointer to the current texture. */
	Texture* m_texturePtr;
	/** 3-element array of the current screen points. */
	vec3f m_screenPoints[3];
	vec3f m_screenNormals[3];
	/** Vector of all pixel fragments in the current triangle. **/
	std::vector<PixelData> m_pixelBuffer;
	/** Pointer to the current mesh. **/
	Mesh* m_currentMesh                              = nullptr;
	Triangle* m_currentTriangle                      = nullptr;
	std::shared_ptr<RenderSettings> m_renderSettings = nullptr;

public:
	ScanlineRenderPipeline() = default;

	bool init(void* windowHandle) override;
	void beginDraw() override;
	void draw() override;
	void addRenderable(IRenderable* renderable) override;
	void endDraw() override;
	void shutdown() override {}
	void resize(int32 width, int32 height) override;

	/** Geometry drawing **/

	bool vertexStage();
	void rasterStage();
	void fragmentStage() const;

	void drawTriangle(Vertex* vertex);
	void drawWireframe() const;
	void drawNormal();
	void drawGrid(Grid* grid) override;

	/** General drawing **/
	void drawLine(const vec3f& inA, const vec3f& inB, const Color& color) override;
	void computeLinePixels(const vec3f& inA, const vec3f& inB, std::vector<vec2f>& points) const;

	uint8* getFrameData() override;
	void setViewData(ViewData* newViewData) override;
	void setRenderSettings(RenderSettings* newRenderSettings) override;
};
