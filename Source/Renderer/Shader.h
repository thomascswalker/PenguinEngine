#pragma once
#pragma warning(disable : 4244)

#include <string>

#include "Core/Buffer.h"
#include "Core/Types.h"

enum class EShaderType : uint8
{
	VertexShader,
	PixelShader
};

class Shader
{
protected:
	EShaderType m_resourceType;
	std::string m_fileName;
	char* m_byteCode;
	size_t m_byteCodeSize;

public:
	virtual ~Shader() = default;

	Shader(const EShaderType resourceType)
		: m_resourceType(resourceType), m_byteCode(nullptr), m_byteCodeSize(0) {}

	[[nodiscard]] EShaderType getResourceType() const
	{
		return m_resourceType;
	}

	[[nodiscard]] std::string getFileName() const
	{
		return m_fileName;
	}

	void setFileName(const std::string& fileName)
	{
		m_fileName = fileName;
	}

	[[nodiscard]] char* getByteCode() const
	{
		return m_byteCode;
	}

	void setByteCode(const void* byteCode, const size_t byteCodeSize)
	{
		m_byteCode = PlatformMemory::malloc<char>(byteCodeSize);
		std::memcpy(m_byteCode, byteCode, byteCodeSize);
		m_byteCodeSize = byteCodeSize;
	}

	[[nodiscard]] size_t getByteCodeSize() const
	{
		return m_byteCodeSize;
	}
};

class VertexShader : public Shader
{
public:
	VertexShader()
		: Shader(EShaderType::VertexShader) {}
};

class PixelShader : public Shader
{
public :
	PixelShader()
		: Shader(EShaderType::PixelShader) {}
};
