#pragma once

#include "Input/InputHandler.h"

class Win32InputHandler : public IInputHandler
{
protected:
	static Win32InputHandler* m_instance;
	Win32InputHandler()  = default;
	~Win32InputHandler() = default;

public:
	static Win32InputHandler* getInstance();

	bool onMouseDown(const EMouseButtonType buttonType, const vec2f& cursorPosition) override
	{
		switch (buttonType)
		{
		case EMouseButtonType::Left: m_mouseLeftDown = true;
			m_onMouseLeftDown.broadcast(cursorPosition);
			break;
		case EMouseButtonType::Right: m_mouseRightDown = true;
			m_onMouseRightDown.broadcast(cursorPosition);
			break;
		case EMouseButtonType::Middle: m_mouseMiddleDown = true;
			m_onMouseMiddleDown.broadcast(cursorPosition);
			break;
		case EMouseButtonType::Invalid:
		default: return false;
		}

		m_clickPosition = cursorPosition;

		return true;
	}

	bool onMouseUp(const EMouseButtonType buttonType, const vec2f& cursorPosition) override
	{
		switch (buttonType)
		{
		case EMouseButtonType::Left: m_mouseLeftDown = false;
			m_onMouseLeftUp.broadcast(cursorPosition);
			break;
		case EMouseButtonType::Right: m_mouseRightDown = false;
			m_onMouseRightUp.broadcast(cursorPosition);
			break;
		case EMouseButtonType::Middle: m_mouseMiddleDown = false;
			m_onMouseMiddleUp.broadcast(cursorPosition);
			break;
		case EMouseButtonType::Invalid:
		default: return false;
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
		m_currentCursorPosition  = cursorPosition;
		m_onMouseMoved.broadcast(m_currentCursorPosition);
		m_deltaCursorPosition = m_currentCursorPosition - m_previousCursorPosition;
		return true;
	}

	bool isMouseDown(const EMouseButtonType buttonType) const override
	{
		switch (buttonType)
		{
		case EMouseButtonType::Left: return m_mouseLeftDown;
		case EMouseButtonType::Right: return m_mouseRightDown;
		case EMouseButtonType::Middle: return m_mouseMiddleDown;
		case EMouseButtonType::Invalid:
		default: return false;
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
		case EMenuAction::LoadModel: break;
		case EMenuAction::LoadTexture: break;
		case EMenuAction::Quit: break;
		}
	}
};
