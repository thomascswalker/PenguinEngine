#pragma warning(disable : 4244)

#include <codecvt>
#include <windowsx.h>

#include "Core/ErrorCodes.h"
#include "Engine/Engine.h"
#include "Importers/TextureImporter.h"
#include "Platforms/Windows/Win32.h"
#include "Renderer/Pipeline/D3D11.h"

/**
 * @brief Default constructor for a new Win32Application.
 * @param newInstance The pointer to the new Win32 instance.
 */
inline Win32Application::Win32Application(HINSTANCE hInstance) : m_hInstance(hInstance)
{
	// Register the window class.
	WNDCLASS windowClass = {};

	windowClass.lpfnWndProc = appWindowProc;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = Win32Window::m_windowClass;

	// Registering the window class
	if (!RegisterClass(&windowClass))
	{
		LOG_ERROR("Failed to register Windows application: {}", getLastErrorAsString())
	}
}

Win32Application* Win32Application::create(HINSTANCE hInstance)
{
	g_windowsApplication = new Win32Application(hInstance);
	return g_windowsApplication;
}

void Win32Application::initialize(Engine* engine)
{
	m_isRunning = true;
	// Forces g_engine to not turn into null
	g_engine = engine;
	g_engine->initialize(this);
	setupInput();
}

void Win32Application::tick(float deltaTime)
{
	CHECK(g_engine);

	// Tick the engine
	g_engine->tick(deltaTime);

	// Process Windows messages
	processMessages();

	if (m_windows.size() == 0)
	{
		m_isRunning = false;
	}
}

int32 Win32Application::exec()
{
	TimePoint startTime;
	TimePoint endTime;
	while (getIsRunning())
	{
		startTime = PTimer::now();
		float deltaTime = std::chrono::duration_cast<DurationMs>(endTime - startTime).count();
		tick(deltaTime);
		endTime = PTimer::now();
	}

	return 0;
}

void Win32Application::setupInput()
{
	LOG_INFO("Setting up input.")

	// Keyboard
	m_keyPressed.addRaw(g_engine, &Engine::onKeyPressed);

	// Mouse
	m_onMouseMiddleScrolled.addRaw(g_engine, &Engine::onMouseMiddleScrolled);
	m_onMouseLeftDown.addRaw(g_engine, &Engine::onLeftMouseDown);
	m_onMouseLeftUp.addRaw(g_engine, &Engine::onLeftMouseUp);
	m_onMouseMiddleUp.addRaw(g_engine, &Engine::onMiddleMouseUp);
	m_onMouseMoved.addRaw(g_engine, &Engine::onMouseMoved);
}

std::shared_ptr<GenericWindow> Win32Application::createWindow(const WindowDescription& description, std::shared_ptr<GenericWindow> parent)
{
	std::shared_ptr<Win32Window> window = std::make_shared<Win32Window>();
	if (!window->initialize(this, m_hInstance, description, dynamic_cast<Win32Window*>(parent.get())))
	{
		LOG_ERROR("Failed to initialize new window: {}", getLastErrorAsString());
		return nullptr;
	}
	window->show();
	m_windows.emplace_back(window);

	if (m_windows.size() == 1)
	{
		m_mainWindow = m_windows[0];
	}
	return m_windows.back();
}

std::shared_ptr<GenericWindow> Win32Application::createWindow(std::shared_ptr<GenericWindow> parent, const std::string& title, const vec2i& size = { g_defaultViewportWidth, g_defaultViewportHeight }, const vec2i& pos = 0)
{
	WindowDescription desc;
	desc.title = title;
	desc.width = size.x;
	desc.height = size.y;
	desc.x = pos.x;
	desc.y = pos.y;
	return createWindow(desc, parent);
}

std::shared_ptr<GenericWindow> Win32Application::getMainWindow()
{
	return m_mainWindow;
}

LRESULT Win32Application::windowProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT						  result = 0;
	std ::shared_ptr<Win32Window> window = getWindowFromHwnd(hwnd);
	switch (msg)
	{
		case WM_TIMER:
			{
				InvalidateRect(hwnd, nullptr, false);
				return 0;
			}
		case WM_CREATE:
			{
				SetTimer(hwnd, g_windowsTimerId, 1, nullptr);
				ShowCursor(TRUE);
				break;
			}
		case WM_CLOSE:
		case WM_DESTROY:
			{
				if (window)
				{
					m_windows.erase(std::remove(m_windows.begin(), m_windows.end(), window));
				}

				DestroyWindow(hwnd);

				return 0;
			}
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_MBUTTONDOWN:
			{
				bool			 mouseUp = false;
				EMouseButtonType buttonType;

				switch (msg)
				{
					case WM_LBUTTONDOWN:
						buttonType = EMouseButtonType::Left;
						break;
					case WM_LBUTTONUP:
						buttonType = EMouseButtonType::Left;
						mouseUp = true;
						break;
					case WM_RBUTTONDOWN:
						buttonType = EMouseButtonType::Right;
						break;
					case WM_RBUTTONUP:
						buttonType = EMouseButtonType::Right;
						mouseUp = true;
						break;
					case WM_MBUTTONUP:
						buttonType = EMouseButtonType::Middle;
						mouseUp = true;
						break;
					case WM_MBUTTONDOWN:
						buttonType = EMouseButtonType::Middle;
						break;
					default:
						return 1;
				}

				const vec2f cursorPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				if (mouseUp)
				{
					onMouseUp(buttonType, cursorPosition);
				}
				else
				{
					onMouseDown(buttonType, cursorPosition);
				}
				break;
			}
		// Mouse movement
		case WM_MOUSEMOVE:
		case WM_INPUT:
			{
				const vec2f cursorPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				onMouseMoved(cursorPosition);
				break;
			}
		case WM_MOUSEWHEEL:
			{
				const float deltaScroll = GET_WHEEL_DELTA_WPARAM(wParam);
				onMouseWheel(-deltaScroll / 120.0f); // Invert delta scroll so rolling forward is positive
				break;
			}
		// Keyboard input
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			{
				const auto key = static_cast<int32>(wParam);
				if (!g_win32KeyMap.contains(key))
				{
					return 0;
				}
				onKeyDown(g_win32KeyMap.at(key), 0, false);
				break;
			}
		case WM_SYSKEYUP:
		case WM_KEYUP:
			{
				const auto key = static_cast<int32>(wParam);
				if (!g_win32KeyMap.contains(key))
				{
					return 0;
				}
				onKeyUp(g_win32KeyMap.at(key), 0, false);
				break;
			}
		case WM_GETMINMAXINFO:
			{
				auto minMaxInfo = (MINMAXINFO*)lParam;
				minMaxInfo->ptMinTrackSize.x = g_minWindowWidth;
				minMaxInfo->ptMinTrackSize.y = g_minWindowHeight;
				minMaxInfo->ptMaxTrackSize.x = g_maxWindowWidth;
				minMaxInfo->ptMaxTrackSize.y = g_maxWindowHeight;
				break;
			}
		case WM_SIZE:
			{
				if (window)
				{
					uint32 width = LOWORD(lParam);
					uint32 height = HIWORD(lParam);
					window->resize(width, height);
				}

				return 0;
			}
		case WM_PAINT:
			{
				if (!window)
				{
					return 0;
				}
				PAINTSTRUCT ps;
				HDC			hdc = BeginPaint(hwnd, &ps);
				window->paint();
				EndPaint(hwnd, &ps);
				break;
			}
		case WM_EXITSIZEMOVE:
		case WM_ERASEBKGND:
			{
				result = 1;
				break;
			}
		default:
			{
				return DefWindowProc(hwnd, msg, wParam, lParam);
			}
	}

	return result;
}

LRESULT Win32Application::appWindowProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
{
	return g_windowsApplication->windowProc(hwnd, msg, wParam, lParam);
}

void Win32Application::processMessages()
{
	MSG message;

	// standard Windows message handling
	while (::PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
	{
		::TranslateMessage(&message);
		::DispatchMessage(&message);
	}
}

std::shared_ptr<Win32Window> Win32Application::getWindowFromHwnd(HWND hwnd)
{
	for (auto& window : m_windows)
	{
		if (window->getHwnd() == hwnd)
		{
			return window;
		}
	}
	return nullptr;
}

bool Win32Application::onMouseDown(const EMouseButtonType buttonType, const vec2f& cursorPosition)
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

bool Win32Application::onMouseUp(const EMouseButtonType buttonType, const vec2f& cursorPosition)
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

bool Win32Application::onMouseWheel(const float delta)
{
	m_mouse.middleDelta = -delta;
	m_onMouseMiddleScrolled.broadcast(m_mouse);
	return true;
}

bool Win32Application::onMouseMoved(const vec2f& cursorPosition)
{
	m_mouse.position = cursorPosition;
	m_previousCursorPosition = m_currentCursorPosition;
	m_currentCursorPosition = cursorPosition;
	m_onMouseMoved.broadcast(m_mouse);
	m_deltaCursorPosition = m_currentCursorPosition - m_previousCursorPosition;
	return true;
}

bool Win32Application::isMouseDown(const EMouseButtonType buttonType) const
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

bool Win32Application::isAnyMouseDown() const
{
	return m_mouse.leftDown || m_mouse.rightDown || m_mouse.middleDown;
}

bool Win32Application::onKeyDown(const EKey keyCode, int32 keyFlags, bool isRepeat)
{
	m_keyStateMap[keyCode] = true;
	return true;
}

bool Win32Application::onKeyUp(const EKey keyCode, int32 keyFlags, bool isRepeat)
{
	m_keyStateMap[keyCode] = false;
	m_keyPressed.broadcast(keyCode);
	return true;
}

bool Win32Application::isKeyDown(const EKey keyCode) const
{
	return m_keyStateMap.at(keyCode);
}
