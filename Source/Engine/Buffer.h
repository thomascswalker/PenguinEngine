#pragma once

#include <vector>

#include "Core/Types.h"

class GenericBuffer
{
protected:
	std::vector<float> m_vertexData;

public:
	GenericBuffer() = default;
	GenericBuffer(const std::vector<float>& vertexData)
		: m_vertexData(vertexData) {}

	virtual void createVertexBuffer(std::vector<float>& data) = 0;
	virtual void createConstantBuffer() = 0;
};
