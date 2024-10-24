#include <cassert>

#include "Painter.h"
#include "Engine/Actors/Camera.h"

void Painter::assertValid()
{
	assert(m_data != nullptr);
}

Painter::Painter(Texture* data, recti viewport)
	: m_data(data), m_viewport(viewport)
{
}

void Painter::drawRect(recti r, Color color, int32 thickness)
{
	assertValid();

	// Color value as int32
	int32 value = color.toInt32();

	// Clamp the rectangle to the current viewport size
	r.clamp(m_viewport);

#if defined(_WIN32) || defined(_WIN64)
	// On Windows we have to flip the Y values because Win32 draws
	// from the bottom up, rather than top down.

	vec2i rmin = r.min();
	vec2i rmax = r.max();
	int32 height = m_viewport.height;

	// Start row
	int32 end = std::clamp(height - rmin.y - g_windowHeightClip, 0, m_viewport.max().y);
	// End row
	int32 start = std::clamp(height - rmax.y - g_windowHeightClip, 0, m_viewport.max().y);

	int32 _thickness = thickness - 1; // True thickness in pixels
	for (int32 row = start; row < end; row++)
	{
		// Pointer to the first pixel in this row with the X offset
		uint32* ptr = m_data->scanline(row) + r.min().x;

		// Fill the entire row if it's either the start or end row,
		// accounting for thickness
		if (row <= (start + thickness) || row >= (end - thickness))
		{
			std::fill(ptr, ptr + (int32)r.width + 1, value);
		}
		// Otherwise only set the pixels on the left and right,
		// accounting for thickness.
		else
		{
			for (int32 i = 0; i < thickness; i++)
			{
				*(ptr + i) = value;					   // Left-side
				*(ptr + ((int32)r.width - i)) = value; // Right-side
			}
		}
	}
#else
	// Start row
	int32 start = std::clamp(r.min().y, 0, m_viewport.max().y);
	// End row
	int32 end = std::clamp(r.max().y, 0, m_viewport.max().y);

	int32 _thickness = thickness - 1; // True thickness in pixels
	for (int32 row = start; row < end; row++)
	{
		// Pointer to the first pixel in this row with the X offset
		uint32* ptr = m_data->scanline(row) + r.min().x;

		// Fill the entire row if it's either the start or end row,
		// accounting for thickness
		if (row <= (start + thickness) || row >= (end - thickness))
		{
			std::fill(ptr, ptr + (int32)r.width + 1, value);
		}
		// Otherwise only set the pixels on the left and right,
		// accounting for thickness.
		else
		{
			for (int32 i = 0; i < thickness; i++)
			{
				*(ptr + i) = value;					   // Left-side
				*(ptr + ((int32)r.width - i)) = value; // Right-side
			}
		}
	}
#endif
}

void Painter::drawRectFilled(recti r, Color color)
{
	assertValid();

	// Color value as int32
	int32 value = color.toInt32();

	// Clamp the rectangle to the current viewport size
	r.clamp(m_viewport);

#if defined(_WIN32) || defined(_WIN64)
	// On Windows we have to flip the Y values because Win32 draws
	// from the bottom up, rather than top down.

	vec2i rmin = r.min();
	vec2i rmax = r.max();
	int32 height = m_viewport.height;

	// Start row
	int32 end = std::clamp(height - rmin.y - g_windowHeightClip, 0, m_viewport.max().y);
	// End row
	int32 start = std::clamp(height - rmax.y - g_windowHeightClip, 0, m_viewport.max().y);

	// Fill each row with the color
	for (int32 row = start; row < end; row++)
	{
		uint32* ptr = m_data->scanline(row) + r.min().x;
		std::fill(ptr, ptr + (int32)r.width, value);
	}
#else
	// Start row
	int32 start = std::clamp(r.min().y, 0, m_viewport.max().y);
	// End row
	int32 end = std::clamp(r.max().y, 0, m_viewport.max().y);

	// Fill each row with the color
	for (int32 row = start; row < end; row++)
	{
		auto ptr = m_data->scanline(row) + r.min().x;
		std::fill(ptr, ptr + (int32)r.width, value);
	}
#endif
}
