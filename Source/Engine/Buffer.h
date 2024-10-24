#pragma once

#include <vector>

#include "Core/Types.h"

struct VertexInputLayout
{
	uint32 count = 0;  // Number of vertexes
	uint32 stride = 0; // Size of each vertex in bytes
};

class GenericBuffer
{
protected:
	VertexInputLayout  m_layout{};
	std::vector<float> m_data;

public:
	GenericBuffer() = default;
	GenericBuffer(const std::vector<float>& vertexData)
		: m_data(vertexData) {}

	virtual void createVertexBuffer(std::vector<float>& data){}
	virtual void createConstantBuffer(int32 byteSize) {}
};
