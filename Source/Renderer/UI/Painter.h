#pragma once

#include <vector>

#include "Core/Types.h"
#include "Math/Color.h"
#include "Math/Rect.h"
#include "Renderer/Texture.h"

class Painter
{
	Texture* m_data = nullptr;
	recti	 m_viewport{};

	void assertValid();

public:
	Painter(Texture* data, recti viewport);

	void setViewport(recti viewport)
	{
		m_viewport = viewport;
	}

	/**
	 * @brief Draws a rectangle with no fill color.
	 */
	virtual void drawRect(recti r, Color color, int32 thickness = 1);

	/**
	 * Draws a rectangle filled with the specified color.
	 */
	virtual void drawRectFilled(recti r, Color color);
};