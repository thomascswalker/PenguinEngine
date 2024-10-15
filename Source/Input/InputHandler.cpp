#include "Input/InputHandler.h"

#if (defined(_WIN32) || defined(_WIN64))
#include "Platforms/Windows/Win32InputHandler.h"
#endif

IInputHandler::IInputHandler()
{
	constexpr uint8 keyCount = static_cast<uint8>(EKey::Count);
	for (uint8 index = 0; index < keyCount; index++)
	{
		m_keyStateMap.emplace(static_cast<EKey>(index), false);
	}
}

IInputHandler* IInputHandler::getInstance()
{
#if (defined(_WIN32) || defined(_WIN64))
	return Win32InputHandler::getInstance();
#else
	return nullptr;
#endif
}

bool IInputHandler::onMouseDown(EMouseButtonType buttonType, const vec2f& cursorPosition)
{
	return false;
}

bool IInputHandler::onMouseUp(EMouseButtonType buttonType, const vec2f& cursorPosition)
{
	return false;
}

bool IInputHandler::onMouseWheel(float delta)
{
	return false;
}

bool IInputHandler::onMouseMoved(const vec2f& cursorPosition)
{
	return false;
}

bool IInputHandler::isMouseDown(EMouseButtonType buttonType) const
{
	return false;
}

bool IInputHandler::isAnyMouseDown() const
{
	return false;
}

vec2f IInputHandler::getClickPosition() const
{
	return m_clickPosition;
}

vec2f IInputHandler::getCurrentCursorPosition() const
{
	return m_currentCursorPosition;
}

vec2f IInputHandler::getPreviousCursorPosition() const
{
	return m_previousCursorPosition;
}

vec2f IInputHandler::getDeltaCursorPosition() const
{
	return m_deltaCursorPosition;
}

void IInputHandler::resetDeltaCursorPosition()
{
	m_deltaCursorPosition.x = 0.0f;
	m_deltaCursorPosition.y = 0.0f;
}

std::vector<EKey> IInputHandler::getKeysDown() const
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

bool IInputHandler::onKeyDown(EKey keyCode, int32 keyFlags, bool isRepeat)
{
	return false;
}

bool IInputHandler::onKeyUp(EKey keyCode, int32 keyFlags, bool isRepeat)
{
	return false;
}

bool IInputHandler::isKeyDown(EKey keyCode) const
{
	return false;
}

void IInputHandler::consumeKey(const EKey keyCode)
{
	m_keyStateMap.at(keyCode) = false;
}

bool IInputHandler::isAltDown() const
{
	return m_keyStateMap.at(EKey::Alt);
}

bool IInputHandler::isShiftDown() const
{
	return m_keyStateMap.at(EKey::Shift);
}

bool IInputHandler::isCtrlDown() const
{
	return m_keyStateMap.at(EKey::Ctrl);
}
