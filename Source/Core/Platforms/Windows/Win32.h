#pragma once

#include <vector>
#include <windows.h>

#include "Engine/Actors/Camera.h"
#include "Engine/Timer.h"
#include "Platforms/Generic/GenericApplication.h"
#include "Win32Window.h"

#define SAFE_DELETE(x)   \
	if (x != nullptr)    \
	{                    \
		DeleteObject(x); \
	}

inline HINSTANCE		 g_hInstance;
inline Win32Application* g_windowsApplication = nullptr;

/* Timer ID for the main window to process WM_TIMER messages. */
constexpr int32 g_windowsTimerId = 1001;

/* Map of the input ASCII key to the corresponding EKey value. */
inline std::map<int32, EKey> g_win32KeyMap{
	{ 'A', EKey::A },
	{ 'B', EKey::B },
	{ 'C', EKey::C },
	{ 'D', EKey::D },
	{ 'E', EKey::E },
	{ 'F', EKey::F },
	{ 'G', EKey::G },
	{ 'H', EKey::H },
	{ 'I', EKey::I },
	{ 'J', EKey::J },
	{ 'K', EKey::K },
	{ 'L', EKey::L },
	{ 'M', EKey::M },
	{ 'N', EKey::N },
	{ 'O', EKey::O },
	{ 'P', EKey::P },
	{ 'Q', EKey::Q },
	{ 'R', EKey::R },
	{ 'S', EKey::S },
	{ 'T', EKey::T },
	{ 'U', EKey::U },
	{ 'V', EKey::V },
	{ 'W', EKey::W },
	{ 'X', EKey::X },
	{ 'Y', EKey::Y },
	{ 'Z', EKey::Z },
	{ VK_ESCAPE, EKey::Escape },
	{ VK_SPACE, EKey::Spacebar },
	{ VK_SHIFT, EKey::Shift },
	{ VK_DELETE, EKey::Backspace },
	{ VK_CONTROL, EKey::Ctrl },
	{ VK_MENU, EKey::Alt },
	{ VK_F1, EKey::F1 },
	{ VK_F2, EKey::F2 },
	{ VK_F3, EKey::F3 },
	{ VK_F4, EKey::F4 },
	{ VK_F5, EKey::F5 },
	{ VK_F6, EKey::F6 },
	{ VK_F7, EKey::F7 },
	{ VK_F8, EKey::F8 },
	{ VK_F9, EKey::F9 },
	{ VK_F10, EKey::F10 },
	{ VK_F11, EKey::F11 },
	{ VK_F12, EKey::F12 },
};

/**
 * @brief Win32 implementation of IApplication.
 */
class Win32Application : public IApplication, public IInputHandler
{
	// Windows specific variables
	std::vector<std::shared_ptr<Win32Window>> m_windows;
	Win32Window*							  m_activeWindow;
	std ::shared_ptr<Win32Window>			  m_mainWindow;
	HINSTANCE								  m_hInstance;
	MouseData								  m_mouse;

protected:
	/**
	 * @brief Default constructor for a new Win32Application.
	 * @param newInstance The pointer to the new Win32 instance.
	 */
	Win32Application(HINSTANCE hInstance);

public:
	// Windows

	static Win32Application* create(HINSTANCE hInstance);

	/* Main window callback processed by all windows. */
	LRESULT		   windowProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);
	static LRESULT appWindowProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);

	void init() override;
	void tick(float deltaTime) override;
	void setupInput() override;

	std::shared_ptr<Win32Window> createWindow(
		const WindowDescription& description, std ::shared_ptr<Win32Window> parent);
	std::shared_ptr<Win32Window> getMainWindow();
	void						 processMessages() override;
	std::shared_ptr<Win32Window> getWindowFromHwnd(HWND hwnd);

	// Input Handler
	bool onMouseDown(const EMouseButtonType buttonType, const vec2f& cursorPosition) override;
	bool onMouseUp(const EMouseButtonType buttonType, const vec2f& cursorPosition) override;
	bool onMouseWheel(const float delta) override;
	bool onMouseMoved(const vec2f& cursorPosition) override;
	bool isMouseDown(const EMouseButtonType buttonType) const override;
	bool isAnyMouseDown() const override;
	bool onKeyDown(const EKey keyCode, int32 keyFlags, bool isRepeat) override;
	bool onKeyUp(const EKey keyCode, int32 keyFlags, bool isRepeat) override;
	bool isKeyDown(const EKey keyCode) const override;
};
