#include "Win32Window.h"
#include "Win32.h"

const TCHAR Win32Window::m_windowClass[] = TEXT("PenguinWindow");

Win32Window::Win32Window() {}

void Win32Window::resize(int32 width, int32 height)
{
	m_description.width = width;
	m_description.height = height;

	m_bitmapInfo.bmiHeader.biWidth = width;
	m_bitmapInfo.bmiHeader.biHeight = -height;
	m_displayBitmap = ::CreateBitmap(width, height, 1, 32, nullptr);

	m_texture = std::make_shared<Texture>(vec2i{ m_description.width, m_description.height });
}

void Win32Window::show()
{
	int32 command = SW_SHOWNORMAL;
	::ShowWindow(m_hwnd, command);
}

void Win32Window::paint()
{
	m_painter->drawRect({ 0, 0, 25, 25 }, Color::red());

	InvalidateRect(m_hwnd, nullptr, TRUE);
	PAINTSTRUCT paint;
	const HDC	deviceContext = BeginPaint(m_hwnd, &paint);
	const HDC	renderContext = CreateCompatibleDC(deviceContext);

	int32 width = m_description.width;
	int32 height = m_description.height;
	SetDIBits(renderContext, m_displayBitmap, 0, height, m_texture->getData(), &m_bitmapInfo, 0);
	if (!BitBlt(deviceContext, 0, 0, m_description.width, m_description.height, renderContext, 0, 0, SRCCOPY)) // NOLINT
	{
		LOG_ERROR("Failed during BitBlt")
	}

	// Cleanup and end painting
	ReleaseDC(m_hwnd, deviceContext);
	DeleteDC(deviceContext);
	EndPaint(m_hwnd, &paint);
}

bool Win32Window::initialize(Win32Application* platform, HINSTANCE instance, const WindowDescription& desc, std ::shared_ptr<Win32Window> parent)
{
	auto flags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	auto parentHwnd = parent != nullptr ? parent->getHwnd() : nullptr;
	m_hwnd = CreateWindowEx(0, m_windowClass, Strings::toWString(m_description.title).c_str(), flags, 0, 0, m_description.width, m_description.height, parentHwnd, nullptr, instance, nullptr);

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

	m_texture = std::make_shared<Texture>(vec2i{ width, height });
	m_painter = std::make_shared<Painter>(m_texture.get(), recti(0, 0, width, height));

	return true;
}

bool Win32Window::create(const HINSTANCE hInstance)
{

	// Create the window.
	m_hwnd =
		CreateWindowEx(0, m_windowClass, Strings::toWString(m_description.title).c_str(), WS_OVERLAPPEDWINDOW, 0, 0, m_description.width, m_description.height, nullptr, nullptr, hInstance, nullptr);

	return m_hwnd != nullptr;
}

LRESULT CALLBACK Win32Window::windowProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
{
	return g_windowsApplication->windowProc(hwnd, msg, wParam, lParam);
}
