#pragma once

#include <memory>
#include <Windows.h>

#include "Core/Macros.h"
#include "Platforms/Windows/Win32Misc.h"
#include "Platforms/Generic/Application.h"
#include "Platforms/Generic/GenericWindow.h"
#include "Renderer/Texture.h"
#include "Renderer/UI/Painter.h"

class Win32Window : public GenericWindow
{
	GENERATE_SUPER(GenericWindow)

	Win32Application* m_platform = nullptr;
	HWND			  m_hwnd = nullptr;
	Win32Window*	  m_parent = nullptr;

	BITMAPINFO				 m_bitmapInfo{};
	HBITMAP					 m_displayBitmap = nullptr;
	std::shared_ptr<Texture> m_displayTexture = nullptr;
	std::shared_ptr<Painter> m_painter = nullptr;

public:
	static const WCHAR m_windowClass[];

public:
	Win32Window();
	void resize(int32 width, int32 height) override;
	void show() override;
	void hide() override;
	void paint() override;
	void paintWidgets(Widget* w);
	void clear() override;
	bool initialize(Win32Application* platform, HINSTANCE hInstance, const WindowDescription& inDescription, Win32Window* parent);

	static LRESULT CALLBACK windowProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);
	HWND					getHwnd() const { return m_hwnd; }

	bool operator==(Win32Window& other) { return m_hwnd == other.m_hwnd; }
	bool operator==(const Win32Window& other) const { return m_hwnd == other.m_hwnd; }
};