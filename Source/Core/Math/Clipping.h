#pragma once

#include "Renderer/Grid.h"
#include "MathFwd.h"
#include "Vector.h"

namespace Clipping
{
	inline bool clipLine(vec2i* a, vec2i* b, const vec2i& size)
	{
		const int32 minX = 0;
		const int32 minY = 0;
		const int32 maxX = size.x - 1;
		const int32 maxY = size.y - 1;

		// Cohen-Sutherland line clipping algorithm
		// Compute region codes for both endpoints
		int32 code1 = (a->x < minX) << 3 | (a->x > maxX) << 2 | (a->y < minY) << 1 | (a->y > maxY);
		int32 code2 = (b->x < minX) << 3 | (b->x > maxX) << 2 | (b->y < minY) << 1 | (b->y > maxY);

		while (code1 || code2)
		{
			// If both endpoints are inside the viewport, exit loop
			if (!(code1 | code2))
			{
				break;
			}

			// If both endpoints are outside the viewport and on the same side, discard the line
			if (code1 & code2)
			{
				return false;
			}

			// Find the endpoint outside the viewport
			const int32 code = code1 ? code1 : code2;
			int32 x, y;

			// Find intersection point using the parametric equation of the line
			if (code & 1)
			{
				// Top edge
				x = a->x + (b->x - a->x) * (maxY - a->y) / (b->y - a->y);
				y = maxY;
			}
			else if (code & 2)
			{
				// Bottom edge
				x = a->x + (b->x - a->x) * (minY - a->y) / (b->y - a->y);
				y = minY;
			}
			else if (code & 4)
			{
				// Right edge
				y = a->y + (b->y - a->y) * (maxX - a->x) / (b->x - a->x);
				x = maxX;
			}
			else
			{
				// Left edge
				y = a->y + (b->y - a->y) * (minX - a->x) / (b->x - a->x);
				x = minX;
			}

			// Update the endpoint
			if (code == code1)
			{
				a->x  = x;
				a->y  = y;
				code1 = (a->x < minX) << 3 | (a->x > maxX) << 2 | (a->y < minY) << 1 | (a->y > maxY);
			}
			else
			{
				b->x  = x;
				b->y  = y;
				code2 = (b->x < minX) << 3 | (b->x > maxX) << 2 | (b->y < minY) << 1 | (b->y > maxY);
			}
		}

		return true;
	}

	inline bool clipLine(linei* line, const vec2i& size)
	{
		return clipLine(&line->a, &line->b, size);
	}

	inline vec3f clipVertex(const vec4f& input, const int32 width, const int32 height)
	{
		// Apply perspective correction
		const vec3f clipPosition{
			input.x / input.w,
			input.y / input.w,
			input.z / input.w
		};

		// Normalized device coordinates
		const vec2f normalizedPosition{
			(clipPosition.x / 2.0f) + 0.5f,
			(clipPosition.y / 2.0f) + 0.5f,
		};

		// Apply the current render width and height
		return vec3f{
			normalizedPosition.x * static_cast<float>(width),
			normalizedPosition.y * static_cast<float>(height),
			(clipPosition.z + 0.5f) * 0.5f
		};
	}
} // namespace Clipping
