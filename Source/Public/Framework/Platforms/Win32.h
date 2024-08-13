#pragma once

#include <windows.h>

#include "PlatformInterface.h"

/** Timer ID for the main window to process WM_TIMER messages. */
constexpr int32 g_windowsTimerId = 1001;

/** Map of the input ASCII key to the corresponding EKey value. */
inline std::map<int32, EKey> g_win32KeyMap
{
	{'A', EKey::A},
	{'B', EKey::B},
	{'C', EKey::C},
	{'D', EKey::D},
	{'E', EKey::E},
	{'F', EKey::F},
	{'G', EKey::G},
	{'H', EKey::H},
	{'I', EKey::I},
	{'J', EKey::J},
	{'K', EKey::K},
	{'L', EKey::L},
	{'M', EKey::M},
	{'N', EKey::N},
	{'O', EKey::O},
	{'P', EKey::P},
	{'Q', EKey::Q},
	{'R', EKey::R},
	{'S', EKey::S},
	{'T', EKey::T},
	{'U', EKey::U},
	{'V', EKey::V},
	{'W', EKey::W},
	{'X', EKey::X},
	{'Y', EKey::Y},
	{'Z', EKey::Z},
	{VK_ESCAPE, EKey::Escape},
	{VK_SPACE, EKey::Spacebar},
	{VK_SHIFT, EKey::Shift},
	{VK_DELETE, EKey::Backspace},
	{VK_CONTROL, EKey::Ctrl},
	{VK_MENU, EKey::Alt},
	{VK_F1, EKey::F1},
	{VK_F2, EKey::F2},
	{VK_F3, EKey::F3},
	{VK_F4, EKey::F4},
	{VK_F5, EKey::F5},
	{VK_F6, EKey::F6},
	{VK_F7, EKey::F7},
	{VK_F8, EKey::F8},
	{VK_F9, EKey::F9},
	{VK_F10, EKey::F10},
	{VK_F11, EKey::F11},
	{VK_F12, EKey::F12},
};

/**
 * @brief Win32 implementation of IPlatform.
 */
class Win32Platform : public IPlatform
{
	// Windows specific variables

	/** @brief Internal class name of the window. */
	LPCWSTR m_className = L"PenguinWindow";

	/** @brief Window display name. */
	LPCWSTR m_windowName = L"Penguin Renderer";

	/** @brief Internal handle to the window. */
	HWND m_hwnd = nullptr;

	/** @brief Default styling of the window (minimize, maximize, close, etc.) */
	DWORD m_defaultStyle = WS_OVERLAPPEDWINDOW;

	/** @brief Default X position. */
	int32 m_defaultX = CW_USEDEFAULT;

	/** @brief Default Y position. */
	int32 m_defaultY = CW_USEDEFAULT;

	/** @brief Default window width. */
	int32 m_defaultWidth = g_defaultViewportWidth;

	/** @brief Default window height. */
	int32 m_defaultHeight = g_defaultViewportHeight;

	/** @brief Static bitmap info used for drawing to the Win32 window. */
	inline static BITMAPINFO m_bitmapInfo;

	/** @brief Static bitmap for storing the Win32 bitmap memory that's displayed. */
	inline static HBITMAP m_displayBitmap;

	/** @brief Stores initialization state of this platform. */
	bool m_initialized = false;

	/** @brief Pointer to the instance of the Win32 platform window. */
	HINSTANCE m_hInstance;

	/** @brief Pointer to the main menu bar. */
	HMENU m_mainMenu = nullptr;

	/** @brief Pointer to the file menu. */
	HMENU m_fileMenu = nullptr;

	/** @brief Pointer to the display menu. */
	HMENU m_displayMenu = nullptr;

	/**
	 * @brief Registers the main Win32 window and constructs the window with CreateWindowExW.
	 * @return True if the window is registered and constructed, false otherwise.
	 */
	bool Register();

public:
	// Windows

	/**
	 * @brief Default constructor for a new Win32Platform.
	 * @param newInstance The pointer to the new Win32 instance.
	 */
	explicit Win32Platform(const HINSTANCE newInstance) : m_hInstance(newInstance) {}

	/**
	 * @brief Main Win32 callback for handling messages.
	 * @param hwnd The main window handle.
	 * @param msg The Win32 message.
	 * @param wParam Additional message information.
	 * @param lParam Additional message information.
	 * @return The status code of the Win32 callback.
	 */
	static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Platform interface

	///** @copydoc IPlatform::create() */
	int32 create() override;

	/** @copydoc IPlatform::show() */
	int32 show() override;

	/** @copydoc IPlatform::start() */
	int32 start() override;

	/** @copydoc IPlatform::loop() */
	int32 loop() override;

	/** @copydoc IPlatform::paint() */
	int32 paint() override;

	/** @copydoc IPlatform::end() */
	int32 end() override;

	/** @copydoc IPlatform::swapBuffers() */
	int32 swapBuffers() override;

	/** @copydoc IPlatform::isInitialized() */
	[[nodiscard]] bool isInitialized() const override
	{
		return m_initialized;
	}

	/** @copydoc IPlatform::getSize() */
	rectf getSize() override;

	/** @copydoc IPlatform::getPlatformType() */
	EPlatformType getPlatformType() override
	{
		return EPlatformType::Windows;
	}

	/** @copydoc IPlatform::getFileDialog() */
	bool getFileDialog(std::string& outFileName) override;

	/** @copydoc IPlatform::constructMenuBar() */
	void constructMenuBar() override;

	/** @copydoc IPlatform::setMenuItemChecked() */
	void setMenuItemChecked(EMenuAction actionId, bool checkState) override;
};
