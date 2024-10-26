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

inline void Painter::setViewport(recti viewport)
{
	m_viewport = viewport;
}

void Painter::drawLine(vec2i a, vec2i b, const Color& color)
{
	bool isSteep = false;
	if (std::abs(a.x - b.x) < std::abs(a.y - b.y))
	{
		a = vec2i(a.y, a.x);
		b = vec2i(b.y, b.x);
		isSteep = true;
	}

	if (a.x > b.x)
	{
		std::swap(a, b);
	}

	const int32 deltaX = b.x - a.x;
	const int32 deltaY = b.y - a.y;
	const int32 deltaError = std::abs(deltaY) * 2;
	int32		errorCount = 0;

	// https://github.com/ssloy/tinyrenderer/issues/28
	int32 y = a.y;

	if (isSteep)
	{
		for (int32 x = a.x; x < b.x; ++x)
		{
			if (y < 0)
			{
				continue;
			}
			m_data->setPixelFromColor(y, x, color);
			errorCount += deltaError;
			if (errorCount > deltaX)
			{
				y += (b.y > a.y ? 1 : -1);
				errorCount -= deltaX * 2;
			}
		}
	}
	else
	{
		for (int32 x = a.x; x < b.x; ++x)
		{
			if (y < 0)
			{
				continue;
			}
			m_data->setPixelFromColor(x, y, color);
			errorCount += deltaError;
			if (errorCount > deltaX)
			{
				y += (b.y > a.y ? 1 : -1);
				errorCount -= deltaX * 2;
			}
		}
	}
}

void Painter::drawRect(recti r, const Color& color, int32 thickness)
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

void Painter::drawRectFilled(recti r, const Color& color)
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

void Painter::drawBezierCurve(std::vector<vec2i>& points, const Color& color)
{
	assert(points.size() > 2); // Minimum of 3 points to draw a curve
	int32 stepCount = 25;
	float stepInterval = 1.0f / stepCount;

	vec2i* pPoint = &points.front();// Previous point
	vec2i* cPoint = pPoint; // Current point
	int32 limit = points.size() - 3;

	for (int32 i = 0; i <= limit; i++)
	{
		vec2i p0 = *cPoint;
		vec2i p1 = *(cPoint + 1);
		vec2i p2 = *(cPoint + 2);

		for (int32 step = 0; step <= stepCount; step++)
		{
			// Compute t value for this step
			float t = stepInterval * (float)step;

			// Lerp with points [0, 1], [1, 2], and then with [01, 12]
			vec2i p01 = Math::lerp(p0, p1, t);
			vec2i p12 = Math::lerp(p1, p2, t);
			vec2i p012 = Math::lerp(p01, p12, t);

			// Draw a line between the previous point and the current interpolated point
			drawLine(*pPoint, p012, color);

			// Store the most recently-interpolated point
			*pPoint = p012;
		}

		// Increment the current point ptr
		cPoint++;
	}

}
