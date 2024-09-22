#pragma once
#pragma warning(disable : 4244)

#include "Renderer/Texture.h"
#include "Engine/Mesh.h"

enum class EResourceType : uint8
{
	VertexShader,
	PixelShader
};

class Shader
{
protected:
	EResourceType m_resourceType;
	Buffer<uint8> m_byteCode;

public:
	Shader(const EResourceType resourceType)
		: m_resourceType(resourceType) {}

	[[nodiscard]] EResourceType getResourceType() const
	{
		return m_resourceType;
	}
};

class VertexShader : public Shader
{
public:
	VertexShader()
		: Shader(EResourceType::VertexShader) {}
};

class PixelShader : public Shader
{
public :
	PixelShader()
		: Shader(EResourceType::PixelShader) {}
};
