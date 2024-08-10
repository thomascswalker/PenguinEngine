#include <windowsx.h>
#include <codecvt>

#include "Framework/Application.h"
#include "Framework/Platforms/Win32Platform.h"
#include "Framework/Core/ErrorCodes.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Input/InputHandler.h"

constexpr int32 g_windowsTimerId = 1001;

std::map<int32, EKey> g_win32KeyMap
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

LRESULT PWin32Platform::windowProc(const HWND hwnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	LRESULT result = 0;
	PEngine* engine = PEngine::getInstance();
	PRenderer* renderer = engine->getRenderer();
	IInputHandler* inputHandler = IInputHandler::GetInstance();

	switch (msg)
	{
	case WM_CREATE:
		{
			SetTimer(hwnd, g_windowsTimerId, 1, nullptr);
			ShowCursor(TRUE);
			return 0;
		}
	case WM_DESTROY:
		{
			DeleteObject(m_displayBitmap);
			engine->shutdown();
			PostQuitMessage(0);
			return 0;
		}
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_MBUTTONDOWN:
		{
			bool mouseUp = false;
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

			const FVector2 cursorPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(renderer->getHeight() - lParam));
			if (mouseUp)
			{
				inputHandler->OnMouseUp(buttonType, cursorPosition);
			}
			else
			{
				inputHandler->OnMouseDown(buttonType, cursorPosition);
			}
			return 0;
		}

	// Mouse movement
	case WM_MOUSEMOVE:
	case WM_INPUT:
		{
			const FVector2 cursorPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(renderer->getHeight() - lParam));
			inputHandler->OnMouseMove(cursorPosition);
			return 0;
		}
	case WM_MOUSEWHEEL:
		{
			const float deltaScroll = GET_WHEEL_DELTA_WPARAM(wParam);
			inputHandler->OnMouseWheel(-deltaScroll / 120.0f); // Invert delta scroll so rolling forward is positive
			return 0;
		}
	// Keyboard input
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		{
			const int32 key = static_cast<int32>(wParam);
			inputHandler->OnKeyDown(g_win32KeyMap.at(key), 0, false);
			return 0;
		}
	case WM_SYSKEYUP:
	case WM_KEYUP:
		{
			const int32 key = static_cast<int32>(wParam);
			inputHandler->OnKeyUp(g_win32KeyMap.at(key), 0, false);
			return 0;
		}
	case WM_PAINT:
		{
			if (!renderer)
			{
				LOG_WARNING("Renderer is not initialized in AppWindowProc::WM_PAINT")
				break;
			}

			// Get the current window size from the buffer
			const std::shared_ptr<PChannel> channel = renderer->getColorChannel();
			const int32 width = channel->m_width;
			const int32 height = channel->m_height;

			// Create a bitmap with the current renderer buffer memory the size of the window
			InvalidateRect(hwnd, nullptr, TRUE);
			PAINTSTRUCT paint;
			const HDC deviceContext = BeginPaint(hwnd, &paint);
			const HDC renderContext = CreateCompatibleDC(deviceContext);
			SetDIBits(renderContext, m_displayBitmap, 0, height, channel->m_memory, &m_bitmapInfo, 0);
			SelectObject(renderContext, m_displayBitmap);
			if (!BitBlt(deviceContext, 0, 0, width, height, renderContext, 0, 0, SRCCOPY)) // NOLINT
			{
				LOG_ERROR("Failed during BitBlt")
				result = 1;
			}

			// Display debug text
			if (renderer->getViewport()->GetShowDebugText())
			{
				// Draw text indicating the current FPS
				RECT clientRect;
				GetClientRect(hwnd, &clientRect);
				clientRect.top += 10;
				clientRect.left += 10;

				std::string outputString = renderer->getViewport()->GetDebugText();
				SetTextColor(deviceContext, RGB(255, 255, 0));
				SetBkColor(deviceContext, TRANSPARENT);
				DrawText(
					deviceContext, // DC
					std::wstring(outputString.begin(), outputString.end()).c_str(), // Message
					-1,
					&clientRect, // Client rectangle (the window)
					DT_TOP | DT_LEFT // Drawing options
				);
			}

			// Cleanup and end painting
			ReleaseDC(hwnd, deviceContext);
			DeleteDC(deviceContext);
			DeleteDC(renderContext);
			EndPaint(hwnd, &paint);

			break;
		}
	case WM_SIZE:
		{
			if (!renderer)
			{
				LOG_WARNING("Renderer is not initialized in AppWindowProc::WM_SIZE")
				break;
			}
			const int32 width = LOWORD(lParam);
			const int32 height = HIWORD(lParam);

			// Update the renderer size
			renderer->resize(width, height);
			m_bitmapInfo.bmiHeader.biWidth = engine->getRenderer()->getWidth();
			m_bitmapInfo.bmiHeader.biHeight = engine->getRenderer()->getHeight();

			const HDC deviceContext = GetDC(hwnd);
			const HDC memoryContext = CreateCompatibleDC(deviceContext);

			const int32 memorySize = width * height * g_bytesPerPixel * 4;
			PPlatformMemory::realloc(m_displayBuffer, static_cast<size_t>(memorySize));
			m_displayBitmap = CreateDIBitmap(memoryContext, &m_bitmapInfo.bmiHeader, DIB_RGB_COLORS, m_displayBuffer,
			                                 &m_bitmapInfo, 0);

			break;
		}
	case WM_EXITSIZEMOVE:
	case WM_ERASEBKGND:
		{
			return 1;
		}
	// Timer called every ms to update
	case WM_TIMER:
		{
			InvalidateRect(hwnd, nullptr, FALSE);
			UpdateWindow(hwnd);
			break;
		}
	case WM_COMMAND:
		{
			const auto actionId = static_cast<EMenuAction>(LOWORD(wParam));
			inputHandler->MenuActionPressed.Broadcast(actionId);
		}
	default:
		{
			result = DefWindowProcW(hwnd, msg, wParam, lParam);
			break;
		}
	}

	// If the mouse has not moved, and we move past all other messages, reset the delta cursor
	// position to [0,0]
	inputHandler->ResetDeltaCursorPosition();

	return result;
}


// ReSharper disable CppParameterMayBeConst
bool PWin32Platform::Register()
{
	// Register the window class.
	WNDCLASS windowClass = {};

	windowClass.lpfnWndProc = windowProc;
	windowClass.hInstance = m_hInstance;
	windowClass.lpszClassName = m_className;

	//Registering the window class
	if (!RegisterClass(&windowClass))
	{
		LOG_ERROR("Failed to register class (PWin32Platform::Register).")
		return false;
	}

	// Create the window.
	m_hwnd = CreateWindowExW(
		0,
		m_className,
		m_windowName,
		m_defaultStyle,
		m_defaultX, m_defaultY,
		m_defaultWidth, m_defaultHeight,
		nullptr,
		nullptr,
		m_hInstance,
		nullptr
	);

	m_initialized = m_hwnd != nullptr;

	if (!m_initialized)
	{
		LOG_ERROR("Failed to create window (PWin32Platform::Register).")
		return false;
	}

	LOG_INFO("Registered class.")
	return true;
}

int32 PWin32Platform::create()
{
	m_initialized = Register();
	if (!m_initialized)
	{
		LOG_ERROR("Window failed to initialize (PWin32Platform::Create).")
		return PlatformInitError;
	}

	constructMenuBar();

	return Success;
}

int32 PWin32Platform::show()
{
	if (!m_initialized)
	{
		LOG_ERROR("Window failed to initialize (PWin32Platform::Show).")
		return PlatformShowError; // Show window failure
	}

	LOG_INFO("Showing window.")
	ShowWindow(m_hwnd, 1);
	return Success;
}

int32 PWin32Platform::start()
{
	if (!m_initialized)
	{
		LOG_ERROR("Window failed to initialize (PWin32Platform::Start).")
		return PlatformStartError; // Start failure
	}

	// Construct the engine
	PEngine* engine = PEngine::getInstance();

	// Initialize the engine
	RECT clientRect;
	GetClientRect(m_hwnd, &clientRect);
	engine->startup(clientRect.right, clientRect.bottom);

	// Initialize Win32 members
	const HDC deviceContext = GetDC(m_hwnd);
	const HDC memoryContext = CreateCompatibleDC(deviceContext);

	m_bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bitmapInfo.bmiHeader.biWidth = engine->getRenderer()->getWidth();
	m_bitmapInfo.bmiHeader.biHeight = engine->getRenderer()->getHeight();
	m_bitmapInfo.bmiHeader.biPlanes = 1;
	m_bitmapInfo.bmiHeader.biBitCount = 32;
	m_bitmapInfo.bmiHeader.biCompression = BI_RGB;

	m_displayBitmap = CreateDIBSection(memoryContext, &m_bitmapInfo, DIB_RGB_COLORS,
	                                   reinterpret_cast<void**>(m_displayBuffer), nullptr, 0);

	const int32 memorySize = m_defaultWidth * m_defaultHeight * g_bytesPerPixel * 4;
	PPlatformMemory::realloc(m_displayBuffer, static_cast<size_t>(memorySize));

	// Process all messages and update the window
	LOG_DEBUG("Engine loop start")
	MSG msg = {};
	while (engine->isRunning() && GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		// Loop, converting DeltaTime from milliseconds to seconds
		if (const int32 loopResult = loop())
		{
			LOG_ERROR("Loop failed (PWin32Platform::Start).")
			return loopResult; // Start failure
		}
	}
	LOG_INFO("Ending engine loop.")

	return end();
}

int32 PWin32Platform::loop()
{
	// Tick the engine forward
	if (PEngine* engine = PEngine::getInstance())
	{
		engine->tick();

		// Draw the frame
		if (PRenderer* renderer = engine->getRenderer())
		{
			// Draw the actual frame
			renderer->draw();
			if (m_displayBuffer)
			{
				swap();
			}

			// Return
			return Success;
		}
	}
	return PlatformLoopError;
}

int32 PWin32Platform::paint()
{
	return 0;
}

int32 PWin32Platform::end()
{
	return Success;
}

int32 PWin32Platform::swap()
{
	const PEngine* engine = PEngine::getInstance();
	const PRenderer* renderer = engine->getRenderer();

	// Copy from the color channel into the display buffer
	const void* src = renderer->getColorChannel()->m_memory;
	void* dst = m_displayBuffer;
	const size_t size = renderer->getColorChannel()->getMemorySize();
	memcpy(dst, src, size);
	return 0;
}

FRect PWin32Platform::getSize()
{
	RECT outRect;

	if (GetWindowRect(getHWnd(), &outRect))
	{
		float width = static_cast<float>(outRect.right - outRect.left);
		float height = static_cast<float>(outRect.bottom - outRect.top);
		return {0, 0, width, height};
	}

	LOG_ERROR("Unable to get window size (PWin32Platform::GetSize).")
	return {};
}

bool PWin32Platform::getFileDialog(std::string& outFileName)
{
	OPENFILENAME ofn;
	TCHAR szFile[260];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hwnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = static_cast<LPCWSTR>(L".obj\0*.obj\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (!GetOpenFileName(&ofn))
	{
		return false;
	}
	int32 fileSize = WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, nullptr, 0, nullptr, nullptr);
	auto tmp = new int8[fileSize];
	WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, tmp, fileSize, nullptr, nullptr);
	outFileName = std::string(tmp);
	return true;
}

void PWin32Platform::constructMenuBar()
{
	m_mainMenu = CreateMenu();
	m_fileMenu = CreateMenu();
	m_displayMenu = CreateMenu();

	// File menu
	AppendMenuW(m_mainMenu, MF_POPUP, UINT_PTR(m_fileMenu), L"&File");
	AppendMenuW(m_fileMenu, MF_STRING, UINT_PTR(EMenuAction::Open), L"&Open...");
	AppendMenuW(m_fileMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuW(m_fileMenu, MF_STRING, UINT_PTR(EMenuAction::Quit), L"&Quit");

	// Display menu
	AppendMenuW(m_mainMenu, MF_POPUP, UINT_PTR(m_displayMenu), L"&Display");
	AppendMenuW(m_displayMenu, MF_UNCHECKED, UINT_PTR(EMenuAction::Wireframe), L"&Wireframe");
	AppendMenuW(m_displayMenu, MF_CHECKED, UINT_PTR(EMenuAction::Shaded), L"&Shaded");
	AppendMenuW(m_displayMenu, MF_CHECKED, UINT_PTR(EMenuAction::Depth), L"&Depth");
	AppendMenuW(m_displayMenu, MF_UNCHECKED, UINT_PTR(EMenuAction::Normals), L"&Normals");

	// Add the main menu bar to the window
	SetMenu(m_hwnd, m_mainMenu);
}

void PWin32Platform::setMenuItemChecked(EMenuAction actionId, const bool checkState)
{
	CheckMenuItem(m_displayMenu, UINT_PTR(actionId), checkState ? MF_CHECKED : MF_UNCHECKED);
}
