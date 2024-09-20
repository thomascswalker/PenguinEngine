#pragma once
#include "Framework/Core/Bitmask.h"

enum ERenderFlag : uint8
{
	None      = 0,
	Wireframe = 1 << 0,
	Shaded    = 1 << 2,
	Depth     = 1 << 3,
	Textures  = 1 << 4,
	Lights    = 1 << 5,
	Normals   = 1 << 6
};

DEFINE_BITMASK_OPERATORS(ERenderFlag);

struct RenderSettings
{
private:
	ERenderFlag m_renderFlags = Wireframe;
	bool m_tileRendering      = false;

public:
	RenderSettings()
	{
		m_renderFlags = Shaded | Depth;
	}

	[[nodiscard]] constexpr bool getRenderFlag(const ERenderFlag flag) const
	{
		return (m_renderFlags & flag) == flag;
	}

	void setRenderFlag(const ERenderFlag flag, const bool state)
	{
		uint8 currentFlag = m_renderFlags;
		state ? currentFlag |= flag : currentFlag &= ~flag;
		m_renderFlags = static_cast<ERenderFlag>(currentFlag);
	}

	bool toggleRenderFlag(const ERenderFlag flag)
	{
		const bool state = getRenderFlag(flag); // Flip the state
		setRenderFlag(flag, !state);
		return !state;
	}

	[[nodiscard]] bool getTileRendering() const
	{
		return m_tileRendering;
	}

	void setTileRendering(const bool newState)
	{
		m_tileRendering = newState;
	}

	bool toggleTileRendering()
	{
		m_tileRendering = !m_tileRendering;
		return m_tileRendering;
	}
};
