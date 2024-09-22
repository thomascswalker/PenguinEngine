#pragma warning(disable : 4244)

#include <windowsx.h>
#include <codecvt>

#include "Framework/Application.h"
#include "Framework/Platforms/Win32.h"
#include "Framework/Core/ErrorCodes.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Importers/TextureImporter.h"
#include "Framework/Input/InputHandler.h"
#include "Framework/Renderer/Pipeline/D3D11.h"

LRESULT Win32Platform::windowProc(const HWND hwnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	LRESULT result              = 0;
	Engine* engine              = Engine::getInstance();
	Viewport* viewport          = engine->getViewport();
	IInputHandler* inputHandler = IInputHandler::getInstance();

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
			case WM_LBUTTONDOWN: buttonType = EMouseButtonType::Left;
				break;
			case WM_LBUTTONUP: buttonType = EMouseButtonType::Left;
				mouseUp = true;
				break;
			case WM_RBUTTONDOWN: buttonType = EMouseButtonType::Right;
				break;
			case WM_RBUTTONUP: buttonType = EMouseButtonType::Right;
				mouseUp = true;
				break;
			case WM_MBUTTONUP: buttonType = EMouseButtonType::Middle;
				mouseUp = true;
				break;
			case WM_MBUTTONDOWN: buttonType = EMouseButtonType::Middle;
				break;
			default: return 1;
			}

			const vec2f cursorPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(viewport->getHeight() - lParam));
			if (mouseUp)
			{
				inputHandler->onMouseUp(buttonType, cursorPosition);
			}
			else
			{
				inputHandler->onMouseDown(buttonType, cursorPosition);
			}
			return 0;
		}

	// Mouse movement
	case WM_MOUSEMOVE:
	case WM_INPUT:
		{
			const vec2f cursorPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(viewport->getHeight() - lParam));
			inputHandler->onMouseMove(cursorPosition);
			return 0;
		}
	case WM_MOUSEWHEEL:
		{
			const float deltaScroll = GET_WHEEL_DELTA_WPARAM(wParam);
			inputHandler->onMouseWheel(-deltaScroll / 120.0f); // Invert delta scroll so rolling forward is positive
			return 0;
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
			inputHandler->onKeyDown(g_win32KeyMap.at(key), 0, false);
			return 0;
		}
	case WM_SYSKEYUP:
	case WM_KEYUP:
		{
			const auto key = static_cast<int32>(wParam);
			if (!g_win32KeyMap.contains(key))
			{
				return 0;
			}
			inputHandler->onKeyUp(g_win32KeyMap.at(key), 0, false);
			return 0;
		}
	case WM_PAINT:
		{
			if (!viewport)
			{
				Logging::warning("Renderer.");
				LOG_WARNING("Renderer is not initialized in AppWindowProc::WM_PAINT")
				break;
			}

			// Get the current window compressedSize from the buffer
			// const std::shared_ptr<Channel> channel = renderer->getColorChannel();
			const int32 width  = viewport->getWidth();
			const int32 height = viewport->getHeight();

			// Create a bitmap with the current renderer buffer memory the compressedSize of the window
			InvalidateRect(hwnd, nullptr, TRUE);
			PAINTSTRUCT paint;
			const HDC deviceContext = BeginPaint(hwnd, &paint);
			const HDC renderContext = CreateCompatibleDC(deviceContext);

			void* colorBuffer = viewport->getRenderPipeline()->getFrameData();
			SetDIBits(renderContext, m_displayBitmap, 0, height, colorBuffer, &m_bitmapInfo, 0); // channel->memory
			SelectObject(renderContext, m_displayBitmap);
			if (!BitBlt(deviceContext, 0, 0, width, height, renderContext, 0, 0, SRCCOPY)) // NOLINT
			{
				LOG_ERROR("Failed during BitBlt")
				result = 1;
			}

			// Display debug text
			if (viewport->getShowDebugText())
			{
				// Draw text indicating the current FPS
				RECT clientRect;
				GetClientRect(hwnd, &clientRect);
				clientRect.top += 10;
				clientRect.left += 10;

				std::string outputString = viewport->getDebugText();
				SetTextColor(deviceContext, RGB(255, 255, 0));
				SetBkColor(deviceContext, TRANSPARENT);
				DrawText(
					deviceContext,                                                  // DC
					std::wstring(outputString.begin(), outputString.end()).c_str(), // Message
					-1,
					&clientRect,     // Client rectangle (the window)
					DT_TOP | DT_LEFT // Drawing options
				);
			}

			// Cleanup and end painting
			ReleaseDC(hwnd, deviceContext);
			ReleaseDC(hwnd, renderContext);
			DeleteDC(deviceContext);
			DeleteDC(renderContext);
			EndPaint(hwnd, &paint);

			break;
		}
	case WM_SIZE:
		{
			if (!viewport)
			{
				LOG_WARNING("Renderer is not initialized in Win32Platform::windowProc::WM_SIZE")
				break;
			}
			const int32 width  = LOWORD(lParam);
			const int32 height = HIWORD(lParam);

			// Update the renderer compressedSize
			viewport->resize(width, height);
			LOG_DEBUG("Resized renderer to [{}, {}].", width, height)

			m_bitmapInfo.bmiHeader.biWidth  = width;
			m_bitmapInfo.bmiHeader.biHeight = height;

			// Create a new empty bitmap with the updated width and height
			m_displayBitmap = CreateBitmap(width, height, 1, 32, nullptr);

			return 0;
		}
	case WM_GETMINMAXINFO:
		{
			auto minMaxInfo              = (MINMAXINFO*)lParam;
			minMaxInfo->ptMinTrackSize.x = g_minWindowWidth;
			minMaxInfo->ptMinTrackSize.y = g_minWindowHeight;
			minMaxInfo->ptMaxTrackSize.x = g_maxWindowWidth;
			minMaxInfo->ptMaxTrackSize.y = g_maxWindowHeight;
			return 0;
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
			inputHandler->m_menuActionPressed.broadcast(actionId);
			break;
		}
	default:
		{
			result = DefWindowProcW(hwnd, msg, wParam, lParam);
			break;
		}
	}

	// If the mouse has not moved, and we move past all other messages, reset the delta cursor
	// position to [0,0]
	inputHandler->resetDeltaCursorPosition();

	return result;
}

// ReSharper disable CppParameterMayBeConst
bool Win32Platform::Register()
{
	// Register the window class.
	WNDCLASS windowClass = {};

	windowClass.lpfnWndProc   = windowProc;
	windowClass.hInstance     = m_hInstance;
	windowClass.lpszClassName = m_className;

	// Registering the window class
	if (!RegisterClass(&windowClass))
	{
		LOG_ERROR("Failed to register class (Win32Platform::Register).")
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
		nullptr);

	m_initialized = m_hwnd != nullptr;

	if (!m_initialized)
	{
		LOG_ERROR("Failed to create window (Win32Platform::Register).")
		return false;
	}

	LOG_INFO("Registered class.")
	return true;
}

int32 Win32Platform::create()
{
	m_initialized = Register();
	if (!m_initialized)
	{
		LOG_ERROR("Window failed to initialize (Win32Platform::Create).")
		return PlatformInitError;
	}

	constructMenuBar();

	// Construct the engine
	Engine* engine = Engine::getInstance();

	// Initialize the engine
	RECT clientRect;
	GetClientRect(m_hwnd, &clientRect);
	engine->startup(clientRect.right, clientRect.bottom);

	// Fill the default bitmap info
	m_bitmapInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	m_bitmapInfo.bmiHeader.biWidth       = g_defaultViewportWidth;
	m_bitmapInfo.bmiHeader.biHeight      = g_defaultViewportHeight;
	m_bitmapInfo.bmiHeader.biPlanes      = 1;
	m_bitmapInfo.bmiHeader.biBitCount    = 32;
	m_bitmapInfo.bmiHeader.biCompression = BI_RGB;

	// Create an empty bitmap which we'll use to display on the window
	m_displayBitmap = CreateBitmap(m_defaultWidth, m_defaultHeight, 1, 32, nullptr);

	return Success;
}

int32 Win32Platform::show()
{
	if (!m_initialized)
	{
		LOG_ERROR("Window failed to initialize (Win32Platform::Show).")
		return PlatformShowError; // Show window failure
	}

	LOG_INFO("Showing window.")
	ShowWindow(m_hwnd, 1);
	return Success;
}

int32 Win32Platform::start()
{
	if (!m_initialized)
	{
		LOG_ERROR("Window failed to initialize (Win32Platform::Start).")
		return PlatformStartError; // Start failure
	}

	// Process all messages and update the window
	LOG_DEBUG("Engine loop start")
	MSG msg = {};

	Engine* engine = Engine::getInstance();
	while (engine->isRunning() && GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		// Process messages prior to running the main loop
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		// Loop, converting DeltaTime from milliseconds to seconds
		if (const int32 loopResult = loop())
		{
			LOG_ERROR("Loop failed (Win32Platform::Start).")
			return loopResult; // Start failure
		}
	}
	LOG_INFO("Ending engine loop.")

	return end();
}

int32 Win32Platform::loop()
{
	// Tick the engine forward
	if (Engine* engine = Engine::getInstance())
	{
		engine->tick();

		// Draw the frame
		if (Viewport* viewport = engine->getViewport())
		{
			// Draw the actual frame
			viewport->draw();

			// Return
			return Success;
		}
	}
	return PlatformLoopError;
}

int32 Win32Platform::paint()
{
	return 0;
}

int32 Win32Platform::end()
{
	return Success;
}

int32 Win32Platform::swapBuffers()
{
	return 0;
}

bool Win32Platform::isInitialized() const
{
	return m_initialized;
}

rectf Win32Platform::getSize()
{
	RECT outRect;

	if (GetWindowRect(m_hwnd, &outRect))
	{
		float width  = static_cast<float>(outRect.right - outRect.left);
		float height = static_cast<float>(outRect.bottom - outRect.top);
		return {0, 0, width, height};
	}

	LOG_ERROR("Unable to get window compressedSize (Win32Platform::GetSize).")
	return {};
}

bool Win32Platform::getFileDialog(std::string& outFileName, const std::string& filter)
{
	OPENFILENAME ofn       = {0};
	TCHAR szFile[MAX_PATH] = {0};
	szFile[0]              = '\0';

	std::string fmtFilter = filter;
	fmtFilter.push_back('\0');
	fmtFilter.append(std::format("*.{}", filter));
	fmtFilter.push_back('\0');
	std::wstring wFilter = Strings::toWString(fmtFilter);

	SecureZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize     = sizeof(ofn);
	ofn.hwndOwner       = m_hwnd;
	ofn.lpstrFile       = szFile;
	ofn.nMaxFile        = sizeof(szFile);
	ofn.lpstrFilter     = wFilter.c_str();
	ofn.nFilterIndex    = 1;
	ofn.lpstrFileTitle  = nullptr;
	ofn.lpstrTitle      = TEXT("Load a file.");
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (!GetOpenFileName(&ofn))
	{
		return false;
	}
	int32 fileSize = WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, nullptr, 0, nullptr, nullptr);
	auto tmp       = new int8[fileSize];
	WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, tmp, fileSize, nullptr, nullptr);
	outFileName = std::string(tmp);

	return true;
}

void Win32Platform::constructMenuBar()
{
	m_mainMenu    = CreateMenu();
	m_fileMenu    = CreateMenu();
	m_displayMenu = CreateMenu();

	// File menu
	AppendMenuW(m_mainMenu, MF_POPUP, UINT_PTR(m_fileMenu), L"&File");
	AppendMenuW(m_fileMenu, MF_STRING, UINT_PTR(EMenuAction::LoadModel), L"&Load Model...");
	AppendMenuW(m_fileMenu, MF_STRING, UINT_PTR(EMenuAction::LoadTexture), L"&Load Texture...");
	AppendMenuW(m_fileMenu, MF_SEPARATOR, 0, nullptr);
	AppendMenuW(m_fileMenu, MF_STRING, UINT_PTR(EMenuAction::Quit), L"&Quit");

	// Display menu
	AppendMenuW(m_mainMenu, MF_POPUP, UINT_PTR(m_displayMenu), L"&Display");
	AppendMenuW(m_displayMenu, MF_UNCHECKED, UINT_PTR(EMenuAction::Wireframe), L"&Wireframe");
	AppendMenuW(m_displayMenu, MF_CHECKED, UINT_PTR(EMenuAction::Shaded), L"&Shaded");
	AppendMenuW(m_displayMenu, MF_CHECKED, UINT_PTR(EMenuAction::Depth), L"&Depth");
	AppendMenuW(m_displayMenu, MF_UNCHECKED, UINT_PTR(EMenuAction::Normals), L"&Normals");
	AppendMenuW(m_displayMenu, MF_SEPARATOR, 0, nullptr);
	AppendMenuW(m_displayMenu, MF_UNCHECKED, UINT_PTR(EMenuAction::TileRendering), L"&Tile Rendering");

	// Add the main menu bar to the window
	SetMenu(m_hwnd, m_mainMenu);
}

void Win32Platform::setMenuItemChecked(EMenuAction actionId, const bool checkState)
{
	CheckMenuItem(m_displayMenu, UINT_PTR(actionId), checkState ? MF_CHECKED : MF_UNCHECKED);
}

void Win32Platform::messageBox(const std::string& title, const std::string& message)
{
	MessageBoxW(
		nullptr,                             // Handle
		Strings::toWString(title).c_str(),   // Title
		Strings::toWString(message).c_str(), // Text
		MB_ICONINFORMATION);                 // Type
}
