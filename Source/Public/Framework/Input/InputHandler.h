// ReSharper disable CppInconsistentNaming
#pragma once

#include <map>

#include "Framework/Core/Bitmask.h"
#include "Math/Vector.h"
#include "Framework/Engine/Delegate.h"

class IInputHandler;
class Win32InputHandler;
class MacOSInputHandler;
class LinuxInputHandler;

enum class EMouseButtonType : uint8
{
	Invalid,
	Left,
	Right,
	Middle
};

enum class EKey : uint8
{
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	One,
	Two,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eight,
	Nine,
	Zero,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	Escape,
	Spacebar,
	Enter,
	Ctrl,
	Shift,
	Alt,
	CapsLock,
	Backspace,

	// Meta
	Count
};

enum class EModifierKey : uint8
{
	None = 0x00,
	Shift = 0x01,
	Ctrl = 0x02,
	Alt = 0x04
};

DEFINE_BITMASK_OPERATORS(EModifierKey)

struct Key
{
	const char* m_name;
};

enum class EMenuAction : uint16
{
	// File
	LoadModel,
	LoadTexture,
	Quit,

	// Display
	Wireframe,
	Shaded,
	Depth,
	Normals,
	VertexNormals
};

DECLARE_MULTICAST_DELEGATE(OnMouseMoved, const vec2f&);
DECLARE_MULTICAST_DELEGATE(OnMouseLeftDown, const vec2f&);
DECLARE_MULTICAST_DELEGATE(OnMouseLeftUp, const vec2f&);
DECLARE_MULTICAST_DELEGATE(OnMouseRightDown, const vec2f&);
DECLARE_MULTICAST_DELEGATE(OnMouseRightUp, const vec2f&);
DECLARE_MULTICAST_DELEGATE(OnMouseMiddleDown, const vec2f&);
DECLARE_MULTICAST_DELEGATE(OnMouseMiddleUp, const vec2f&);

DECLARE_MULTICAST_DELEGATE(OnKeyPressed, EKey);
DECLARE_MULTICAST_DELEGATE(OnMouseMiddleScrolled, float);

DECLARE_MULTICAST_DELEGATE(OnMenuActionPressed, EMenuAction);

class IInputHandler
{
public:
	static IInputHandler* getInstance();

	// Events

protected:
	bool m_mouseLeftDown = false;
	bool m_mouseRightDown = false;
	bool m_mouseMiddleDown = false;

	vec2f m_clickPosition;
	vec2f m_currentCursorPosition;
	vec2f m_previousCursorPosition;
	vec2f m_deltaCursorPosition;

	std::map<EKey, bool> m_keyStateMap;
	EModifierKey		 m_modifierKeys = EModifierKey::None;

	IInputHandler()
	{
		constexpr uint8 keyCount = static_cast<uint8>(EKey::Count);
		for (uint8 index = 0; index < keyCount; index++)
		{
			m_keyStateMap.emplace(static_cast<EKey>(index), false);
		}
	}

	~IInputHandler() = default;

public:
	// Events
	OnMouseMoved		  m_onMouseMoved;
	OnMouseLeftDown		  m_onMouseLeftDown;
	OnMouseRightDown	  m_onMouseRightDown;
	OnMouseMiddleDown	  m_onMouseMiddleDown;
	OnMouseLeftUp		  m_onMouseLeftUp;
	OnMouseRightUp		  m_onMouseRightUp;
	OnMouseMiddleUp		  m_onMouseMiddleUp;
	OnMouseMiddleScrolled m_onMouseMiddleScrolled;
	OnKeyPressed		  m_keyPressed;
	OnMenuActionPressed	  m_menuActionPressed;

	// Mouse
	virtual bool onMouseDown(EMouseButtonType buttonType, const vec2f& cursorPosition)
	{
		return false;
	}

	virtual bool onMouseUp(EMouseButtonType buttonType, const vec2f& cursorPosition)
	{
		return false;
	}

	virtual bool onMouseWheel(float delta)
	{
		return false;
	}

	virtual bool onMouseMove(const vec2f& cursorPosition)
	{
		return false;
	}

	virtual bool isMouseDown(EMouseButtonType buttonType) const
	{
		return false;
	}

	virtual bool isAnyMouseDown() const
	{
		return false;
	}

	virtual vec2f getClickPosition() const
	{
		return m_clickPosition;
	}

	virtual vec2f getCurrentCursorPosition() const
	{
		return m_currentCursorPosition;
	}

	virtual vec2f getPreviousCursorPosition() const
	{
		return m_previousCursorPosition;
	}

	virtual vec2f getDeltaCursorPosition() const
	{
		return m_deltaCursorPosition;
	}

	virtual void resetDeltaCursorPosition()
	{
		m_deltaCursorPosition.x = 0.0f;
		m_deltaCursorPosition.y = 0.0f;
	}

	// Keys
	virtual std::vector<EKey> getKeysDown() const
	{
		std::vector<EKey> keys;
		for (const auto& [k, v] : m_keyStateMap)
		{
			if (v)
			{
				keys.emplace_back(k);
			}
		}
		return keys;
	}

	virtual bool onKeyDown(EKey keyCode, int32 keyFlags, bool isRepeat)
	{
		return false;
	}

	virtual bool onKeyUp(EKey keyCode, int32 keyFlags, bool isRepeat)
	{
		return false;
	}

	virtual bool isKeyDown(EKey keyCode) const
	{
		return false;
	}

	virtual void consumeKey(const EKey keyCode)
	{
		m_keyStateMap.at(keyCode) = false;
	}

	virtual bool isAltDown() const
	{
		return m_keyStateMap.at(EKey::Alt);
	}

	virtual bool isShiftDown() const
	{
		return m_keyStateMap.at(EKey::Shift);
	}

	virtual bool isCtrlDown() const
	{
		return m_keyStateMap.at(EKey::Ctrl);
	}

	virtual void onMenuActionPressed(EMenuAction actionId) = 0;
};

class Win32InputHandler : public IInputHandler
{
protected:
	static Win32InputHandler* m_instance;
	Win32InputHandler() = default;
	~Win32InputHandler() = default;

public:
	static Win32InputHandler* getInstance();

	bool onMouseDown(const EMouseButtonType buttonType, const vec2f& cursorPosition) override
	{
		switch (buttonType)
		{
			case EMouseButtonType::Left:
				m_mouseLeftDown = true;
				m_onMouseLeftDown.broadcast(cursorPosition);
				break;
			case EMouseButtonType::Right:
				m_mouseRightDown = true;
				m_onMouseRightDown.broadcast(cursorPosition);
				break;
			case EMouseButtonType::Middle:
				m_mouseMiddleDown = true;
				m_onMouseMiddleDown.broadcast(cursorPosition);
				break;
			case EMouseButtonType::Invalid:
			default:
				return false;
		}

		m_clickPosition = cursorPosition;

		return true;
	}

	bool onMouseUp(const EMouseButtonType buttonType, const vec2f& cursorPosition) override
	{
		switch (buttonType)
		{
			case EMouseButtonType::Left:
				m_mouseLeftDown = false;
				m_onMouseLeftUp.broadcast(cursorPosition);
				break;
			case EMouseButtonType::Right:
				m_mouseRightDown = false;
				m_onMouseRightUp.broadcast(cursorPosition);
				break;
			case EMouseButtonType::Middle:
				m_mouseMiddleDown = false;
				m_onMouseMiddleUp.broadcast(cursorPosition);
				break;
			case EMouseButtonType::Invalid:
			default:
				return false;
		}
		m_clickPosition = 0;

		return true;
	}

	bool onMouseWheel(const float delta) override
	{
		// Invert delta
		m_onMouseMiddleScrolled.broadcast(delta);
		return true;
	}

	bool onMouseMove(const vec2f& cursorPosition) override
	{
		// Update current cursor position
		if (m_currentCursorPosition == cursorPosition)
		{
			m_deltaCursorPosition = 0;
			return false;
		}
		m_previousCursorPosition = m_currentCursorPosition;
		m_currentCursorPosition = cursorPosition;
		m_onMouseMoved.broadcast(m_currentCursorPosition);
		m_deltaCursorPosition = m_currentCursorPosition - m_previousCursorPosition;
		return true;
	}

	bool isMouseDown(const EMouseButtonType buttonType) const override
	{
		switch (buttonType)
		{
			case EMouseButtonType::Left:
				return m_mouseLeftDown;
			case EMouseButtonType::Right:
				return m_mouseRightDown;
			case EMouseButtonType::Middle:
				return m_mouseMiddleDown;
			case EMouseButtonType::Invalid:
			default:
				return false;
		}
	}

	bool isAnyMouseDown() const override
	{
		return m_mouseLeftDown || m_mouseRightDown || m_mouseMiddleDown;
	}

	bool onKeyDown(const EKey keyCode, int32 keyFlags, bool isRepeat) override
	{
		m_keyStateMap[keyCode] = true;
		return true;
	}

	bool onKeyUp(const EKey keyCode, int32 keyFlags, bool isRepeat) override
	{
		m_keyStateMap[keyCode] = false;
		m_keyPressed.broadcast(keyCode);
		return true;
	}

	bool isKeyDown(const EKey keyCode) const override
	{
		return m_keyStateMap.at(keyCode);
	}

	void onMenuActionPressed(const EMenuAction actionId) override
	{
		switch (actionId)
		{
			case EMenuAction::LoadModel:
				break;
			case EMenuAction::LoadTexture:
				break;
			case EMenuAction::Quit:
				break;
		}
	}
};
