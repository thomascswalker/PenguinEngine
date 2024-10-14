#pragma once

#include "Input/InputHandler.h"
#include "Engine/Mouse.h"

class Win32InputHandler : public IInputHandler
{
	MouseData m_mouse;

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
				m_mouse.leftDown = true;
				m_mouse.clickPosition = cursorPosition;
				m_onMouseLeftDown.broadcast(m_mouse);
				break;
			case EMouseButtonType::Right:
				m_mouse.rightDown = true;
				m_mouse.clickPosition = cursorPosition;
				m_onMouseRightDown.broadcast(m_mouse);
				break;
			case EMouseButtonType::Middle:
				m_mouse.middleDown = true;
				m_mouse.clickPosition = cursorPosition;
				m_onMouseMiddleDown.broadcast(m_mouse);
				break;
			case EMouseButtonType::Invalid:
			default:
				return false;
		}

		return true;
	}

	bool onMouseUp(const EMouseButtonType buttonType, const vec2f& cursorPosition) override
	{
		switch (buttonType)
		{
			case EMouseButtonType::Left:
				m_mouse.leftDown = false;
				m_mouse.clickPosition = 0;
				m_onMouseLeftUp.broadcast(m_mouse);
				break;
			case EMouseButtonType::Right:
				m_mouse.rightDown = false;
				m_mouse.clickPosition = 0;
				m_onMouseRightUp.broadcast(m_mouse);
				break;
			case EMouseButtonType::Middle:
				m_mouse.middleDown = false;
				m_mouse.clickPosition = 0;
				m_onMouseMiddleUp.broadcast(m_mouse);
				break;
			case EMouseButtonType::Invalid:
			default:
				return false;
		}

		return true;
	}

	bool onMouseWheel(const float delta) override
	{
		// Invert delta
		m_mouse.middleDelta = delta;
		m_onMouseMiddleScrolled.broadcast(m_mouse);
		return true;
	}

	bool onMouseMoved(const vec2f& cursorPosition) override
	{
		// Update current cursor position
		if (m_currentCursorPosition == cursorPosition)
		{
			m_deltaCursorPosition = 0;
			return false;
		}
		m_mouse.position = cursorPosition;
		m_previousCursorPosition = m_currentCursorPosition;
		m_currentCursorPosition = cursorPosition;
		m_onMouseMoved.broadcast(m_mouse);
		m_deltaCursorPosition = m_currentCursorPosition - m_previousCursorPosition;
		return true;
	}

	bool isMouseDown(const EMouseButtonType buttonType) const override
	{
		switch (buttonType)
		{
			case EMouseButtonType::Left:
				return m_mouse.leftDown;
			case EMouseButtonType::Right:
				return m_mouse.rightDown;
			case EMouseButtonType::Middle:
				return m_mouse.middleDown;
			case EMouseButtonType::Invalid:
			default:
				return false;
		}
	}

	bool isAnyMouseDown() const override
	{
		return m_mouse.leftDown || m_mouse.rightDown || m_mouse.middleDown;
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
