#include "Win32Window.h"
#include "Win32.h"

const TCHAR Win32Window::m_windowClass[] = TEXT("PenguinWindow");

Win32Window::Win32Window() : Super() {}

void Win32Window::resize(int32 width, int32 height)
{
	m_description.width = width;
	m_description.height = height;

	m_bitmapInfo.bmiHeader.biWidth = width;
	m_bitmapInfo.bmiHeader.biHeight = -height;
	m_displayBitmap = ::CreateBitmap(width, height, 1, 32, nullptr);

	// Create a new texture for this window.
	m_displayTexture = std::make_shared<Texture>(vec2i{ m_description.width, m_description.height });

	// Set the painter texture to the new texture we just remade.
	m_painter->setTexture(m_displayTexture.get());
	m_painter->setViewport(recti(0, 0, width, height));
}

void Win32Window::show()
{
	int32 command = SW_SHOWNORMAL;
	::ShowWindow(m_hwnd, command);
}

void Win32Window::hide()
{
	int32 command = SW_HIDE;
	::ShowWindow(m_hwnd, command);
}

void Win32Window::paint()
{
	// Clear to background color first
	clear();

	if (m_canvas != nullptr)
	{
		// Layout all widgets
		LayoutEngine::layoutAllWidgets(m_canvas.get(), recti(0, 0, getWidth(), getHeight()));
		LayoutEngine::updateWidgets(m_canvas.get(), g_windowsApplication->getMouseData());

		// Render all widgets to bitmap
		for (Widget* w : m_canvas->getChildren())
		{
			w->paint(m_painter.get());
		}
	}

	HDC windowDeviceContext;
	HDC memoryDeviceContext = NULL;

	// Retrieve the handle to a display device context for the client
	// area of the window.
	windowDeviceContext = GetDC(m_hwnd);

	// Create a compatible DC, which is used in a BitBlt from the window DC.
	memoryDeviceContext = CreateCompatibleDC(windowDeviceContext);
	if (!memoryDeviceContext)
	{
		MessageBox(m_hwnd, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
		return;
	}

	// Get the client area for size calculation.
	RECT rcClient;
	GetClientRect(m_hwnd, &rcClient);
	int32 width = rcClient.right - rcClient.left;
	int32 height = rcClient.bottom - rcClient.top;

	// This is the best stretch mode.
	SetStretchBltMode(windowDeviceContext, HALFTONE);

	// Create a compatible bitmap from the Window DC.
	m_displayBitmap = CreateCompatibleBitmap(windowDeviceContext, width, height);
	if (!m_displayBitmap)
	{
		MessageBox(m_hwnd, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
		return;
	}

	// Select the compatible bitmap into the compatible memory DC.
	SelectObject(memoryDeviceContext, m_displayBitmap);

	// Bit block transfer into our compatible memory DC.
	StretchDIBits(windowDeviceContext, // Target context
		0, 0,						   // Dest pos
		width, height,				   // Dest size
		0, 0,						   // Source pos
		width, height,				   // Source size
		m_displayTexture->getData(),   // Source data
		&m_bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	SAFE_DELETE(m_displayBitmap)
	SAFE_DELETE(memoryDeviceContext)
	ReleaseDC(m_hwnd, windowDeviceContext);
}

void Win32Window::clear()
{
	m_painter->fill(getBackgroundColor());
}

bool Win32Window::initialize(Win32Application* platform, HINSTANCE instance, const WindowDescription& desc, Win32Window* parent)
{
	auto flags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	auto parentHwnd = parent != nullptr ? parent->getHwnd() : nullptr;
	m_hwnd = CreateWindowEx(0, m_windowClass, Strings::toWString(desc.title).c_str(), flags, 0, 0, desc.width, desc.height, parentHwnd, nullptr, instance, nullptr);

	if (m_hwnd == nullptr)
	{
		LOG_ERROR("Failed to create new window: {}", getLastErrorAsString())
		return false;
	}
	m_platform = platform;
	m_description = desc;

	int32 width = desc.width;
	int32 height = desc.height;

	m_bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bitmapInfo.bmiHeader.biWidth = width;
	m_bitmapInfo.bmiHeader.biHeight = -height; // Negative
	m_bitmapInfo.bmiHeader.biPlanes = 1;
	m_bitmapInfo.bmiHeader.biBitCount = 32;
	m_bitmapInfo.bmiHeader.biCompression = BI_RGB;

	m_displayTexture = std::make_shared<Texture>(vec2i{ width, height });
	m_painter = std::make_shared<Painter>(m_displayTexture.get(), recti(0, 0, width, height));

	return true;
}

LRESULT CALLBACK Win32Window::windowProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
{
	return g_windowsApplication->windowProc(hwnd, msg, wParam, lParam);
}
