#pragma once

#include <windows.h>

#include "PlatformInterface.h"
#include "Framework/Renderer/Viewport.h"

class PWin32Platform : public IPlatform
{
	// Windows specific variables
	LPCWSTR m_className = L"PenguinWindow";
	LPCWSTR m_windowName = L"Penguin Renderer";
	HWND m_hwnd = nullptr;
	DWORD m_defaultStyle = WS_OVERLAPPEDWINDOW;
	int32 m_defaultX = CW_USEDEFAULT;
	int32 m_defaultY = CW_USEDEFAULT;
	int32 m_defaultWidth = g_defaultViewportWidth;
	int32 m_defaultHeight = g_defaultViewportHeight;

	inline static BITMAPINFO m_bitmapInfo;
	inline static HBITMAP m_displayBitmap;
	inline static int8* m_displayBuffer;

	bool m_initialized = false;

	HINSTANCE m_hInstance;
	auto Register() -> bool;

	HMENU m_mainMenu;
	HMENU m_fileMenu;
	HMENU m_displayMenu;

public:
	// Platform interface
	int32 create() override;
	int32 show() override;
	int32 start() override;
	int32 loop() override;
	int32 paint() override;
	int32 end() override;
	int32 swap() override;
	bool isInitialized() const override { return m_initialized; }

	// Windows
	PWin32Platform(const HINSTANCE newInstance) : m_hInstance(newInstance)
	{
	}

	static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	HWND getHWnd() const { return m_hwnd; }
	void setHInstance(const HINSTANCE newInstance) { m_hInstance = newInstance; }
	FRect getSize() override;
	EPlatformType getPlatformType() override { return EPlatformType::Windows; }

	// Menu bar
	bool getFileDialog(std::string& outFileName) override;
	void constructMenuBar() override;
	void setMenuItemChecked(EMenuAction actionId, bool checkState) override;
};
