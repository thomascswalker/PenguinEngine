#include <windowsx.h>
#include <codecvt>

#include "Framework/Application.h"
#include "Framework/Platforms/Win32Platform.h"
#include "Framework/Core/ErrorCodes.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Input/InputHandler.h"

constexpr int32 WINDOWS_TIMER_ID = 1001;

std::map<int32, EKey> Win32KeyMap
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

LRESULT PWin32Platform::WindowProc(HWND Hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = 0;
    PEngine* Engine = PEngine::GetInstance();
    PRenderer* Renderer = Engine->GetRenderer();
    IInputHandler* InputHandler = IInputHandler::GetInstance();

    switch (Msg)
    {
    case WM_CREATE :
        {
            SetTimer(Hwnd, WINDOWS_TIMER_ID, 1, nullptr);
            ShowCursor(TRUE);
            return 0;
        }
    case WM_DESTROY :
        {
            DeleteObject(DisplayBitmap);
            Engine->Shutdown();
            PostQuitMessage(0);
            return 0;
        }
    case WM_LBUTTONDOWN :
    case WM_LBUTTONUP :
    case WM_RBUTTONDOWN :
    case WM_RBUTTONUP :
    case WM_MBUTTONUP :
    case WM_MBUTTONDOWN :
        {
            bool bMouseUp = false;
            EMouseButtonType ButtonType;

            switch (Msg)
            {
            case WM_LBUTTONDOWN :
                ButtonType = EMouseButtonType::Left;
                break;
            case WM_LBUTTONUP :
                ButtonType = EMouseButtonType::Left;
                bMouseUp = true;
                break;
            case WM_RBUTTONDOWN :
                ButtonType = EMouseButtonType::Right;
                break;
            case WM_RBUTTONUP :
                ButtonType = EMouseButtonType::Right;
                bMouseUp = true;
                break;
            case WM_MBUTTONUP :
                ButtonType = EMouseButtonType::Middle;
                bMouseUp = true;
                break;
            case WM_MBUTTONDOWN :
                ButtonType = EMouseButtonType::Middle;
                break;
            default :
                return 1;
            }

            const FVector2 CursorPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(Renderer->GetHeight() - lParam));
            if (bMouseUp)
            {
                InputHandler->OnMouseUp(ButtonType, CursorPosition);
            }
            else
            {
                InputHandler->OnMouseDown(ButtonType, CursorPosition);
            }
            return 0;
        }

    // Mouse movement
    case WM_MOUSEMOVE :
    case WM_INPUT :
        {
            const FVector2 CursorPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(Renderer->GetHeight() - lParam));
            InputHandler->OnMouseMove(CursorPosition);
            return 0;
        }
    case WM_MOUSEWHEEL :
        {
            const float DeltaScroll = GET_WHEEL_DELTA_WPARAM(wParam);
            InputHandler->OnMouseWheel(-DeltaScroll / 120.0f); // Invert delta scroll so rolling forward is positive
            return 0;
        }
    // Keyboard input
    case WM_SYSKEYDOWN :
    case WM_KEYDOWN :
        {
            const int32 Char = static_cast<int32>(wParam);
            InputHandler->OnKeyDown(Win32KeyMap.at(Char), 0, false);
            return 0;
        }
    case WM_SYSKEYUP :
    case WM_KEYUP :
        {
            const int32 Char = static_cast<int32>(wParam);
            InputHandler->OnKeyUp(Win32KeyMap.at(Char), 0, false);
            return 0;
        }
    case WM_PAINT :
        {
            if (!Renderer)
            {
                LOG_WARNING("Renderer is not initialized in AppWindowProc::WM_PAINT")
                break;
            }

            // Get the current window size from the buffer
            const std::shared_ptr<PChannel> Buffer = Renderer->GetColorChannel();
            const int32 Width = Buffer->Width;
            const int32 Height = Buffer->Height;

            // Create a bitmap with the current renderer buffer memory the size of the window
            InvalidateRect(Hwnd, nullptr, TRUE);
            PAINTSTRUCT Paint;
            const HDC DeviceContext = BeginPaint(Hwnd, &Paint);
            const HDC RenderContext = CreateCompatibleDC(DeviceContext);
            SetDIBits(RenderContext, DisplayBitmap, 0, Height, Buffer->Memory, &BitmapInfo, 0);
            SelectObject(RenderContext, DisplayBitmap);
            if (!BitBlt(DeviceContext, 0, 0, Width, Height, RenderContext, 0, 0, SRCCOPY)) // NOLINT
            {
                LOG_ERROR("Failed during BitBlt")
                Result = 1;
            }

            // Display debug text
            if (Renderer->GetViewport()->GetShowDebugText())
            {
                // Draw text indicating the current FPS
                RECT ClientRect;
                GetClientRect(Hwnd, &ClientRect);
                ClientRect.top += 10;
                ClientRect.left += 10;

                std::string OutputString = Renderer->GetViewport()->GetDebugText();
                SetTextColor(DeviceContext, RGB(255, 255, 0));
                SetBkColor(DeviceContext, TRANSPARENT);
                DrawText(
                    DeviceContext,                                                  // DC
                    std::wstring(OutputString.begin(), OutputString.end()).c_str(), // Message
                    -1,
                    &ClientRect,     // Client rectangle (the window)
                    DT_TOP | DT_LEFT // Drawing options
                );
            }

            // Cleanup and end painting
            ReleaseDC(Hwnd, DeviceContext);
            DeleteDC(DeviceContext);
            DeleteDC(RenderContext);
            EndPaint(Hwnd, &Paint);

            break;
        }
    case WM_SIZE :
        {
            if (!Renderer)
            {
                LOG_WARNING("Renderer is not initialized in AppWindowProc::WM_SIZE")
                break;
            }
            const int32 Width = LOWORD(lParam);
            const int32 Height = HIWORD(lParam);

            // Update the renderer size
            Renderer->Resize(Width, Height);
            BitmapInfo.bmiHeader.biWidth = Engine->GetRenderer()->GetWidth();
            BitmapInfo.bmiHeader.biHeight = Engine->GetRenderer()->GetHeight();

            const HDC DeviceContext = GetDC(Hwnd);
            const HDC MemoryContext = CreateCompatibleDC(DeviceContext);

            const int32 MemorySize = Width * Height * BYTES_PER_PIXEL * 4;
            PPlatformMemory::Realloc(DisplayBuffer, static_cast<size_t>(MemorySize));
            DisplayBitmap = CreateDIBitmap(MemoryContext, &BitmapInfo.bmiHeader, DIB_RGB_COLORS, DisplayBuffer, &BitmapInfo, 0);

            break;
        }
    case WM_EXITSIZEMOVE :
    case WM_ERASEBKGND :
        {
            return 1;
        }
    // Timer called every ms to update
    case WM_TIMER :
        {
            InvalidateRect(Hwnd, nullptr, FALSE);
            UpdateWindow(Hwnd);
            break;
        }
    case WM_COMMAND :
        {
            auto ActionId = static_cast<EMenuAction>(LOWORD(wParam));
            InputHandler->MenuActionPressed.Broadcast(ActionId);
        }
    default :
        {
            Result = DefWindowProcW(Hwnd, Msg, wParam, lParam);
            break;
        }
    }

    // If the mouse has not moved, and we move past all other messages, reset the delta cursor
    // position to [0,0]
    InputHandler->ResetDeltaCursorPosition();

    return Result;
}


// ReSharper disable CppParameterMayBeConst
bool PWin32Platform::Register()
{
    // Register the window class.
    WNDCLASS WindowClass = {};

    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = HInstance;
    WindowClass.lpszClassName = ClassName;

    //Registering the window class
    if (!RegisterClass(&WindowClass))
    {
        LOG_ERROR("Failed to register class (PWin32Platform::Register).")
        return false;
    }

    // Create the window.
    Hwnd = CreateWindowExW(
        0,
        ClassName,
        WindowName,
        DefaultStyle,
        DefaultX, DefaultY,
        DefaultWidth, DefaultHeight,
        nullptr,
        nullptr,
        HInstance,
        nullptr
    );

    bInitialized = Hwnd != nullptr;

    if (!bInitialized)
    {
        LOG_ERROR("Failed to create window (PWin32Platform::Register).")
        return false;
    }

    LOG_INFO("Registered class.")
    return true;
}

int32 PWin32Platform::Create()
{
    bInitialized = Register();
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWin32Platform::Create).")
        return PlatformInitError;
    }

    ConstructMenuBar();

    return Success;
}

int32 PWin32Platform::Show()
{
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWin32Platform::Show).")
        return PlatformShowError; // Show window failure
    }

    LOG_INFO("Showing window.")
    ShowWindow(Hwnd, 1);
    return Success;
}

int32 PWin32Platform::Start()
{
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWin32Platform::Start).")
        return PlatformStartError; // Start failure
    }

    // Construct the engine
    PEngine* Engine = PEngine::GetInstance();

    // Initialize the engine
    RECT ClientRect;
    GetClientRect(Hwnd, &ClientRect);
    Engine->Startup(ClientRect.right, ClientRect.bottom);

    // Initialize Win32 members
    const HDC DeviceContext = GetDC(Hwnd);
    const HDC MemoryContext = CreateCompatibleDC(DeviceContext);

    BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    BitmapInfo.bmiHeader.biWidth = Engine->GetRenderer()->GetWidth();
    BitmapInfo.bmiHeader.biHeight = Engine->GetRenderer()->GetHeight();
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    DisplayBitmap = CreateDIBSection(MemoryContext, &BitmapInfo, DIB_RGB_COLORS, reinterpret_cast<void**>(DisplayBuffer), nullptr, 0);

    const int32 MemorySize = DefaultWidth * DefaultHeight * BYTES_PER_PIXEL * 4;
    PPlatformMemory::Realloc(DisplayBuffer, static_cast<size_t>(MemorySize));

    // Process all messages and update the window
    LOG_DEBUG("Engine loop start")
    MSG Msg = {};
    while (Engine->IsRunning() && GetMessage(&Msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);

        // Loop, converting DeltaTime from milliseconds to seconds
        if (const int32 LoopResult = Loop())
        {
            LOG_ERROR("Loop failed (PWin32Platform::Start).")
            return LoopResult; // Start failure
        }
    }
    LOG_INFO("Ending engine loop.")

    return End();
}

int32 PWin32Platform::Loop()
{
    // Tick the engine forward
    if (PEngine* Engine = PEngine::GetInstance())
    {
        Engine->Tick();

        // Draw the frame
        if (PRenderer* Renderer = Engine->GetRenderer())
        {
            // Draw the actual frame
            Renderer->Draw();
            if (DisplayBuffer)
            {
                Swap();
            }

            // Return
            return Success;
        }
    }
    return PlatformLoopError;
}

int32 PWin32Platform::Paint()
{
    return 0;
}

int32 PWin32Platform::End()
{
    return Success;
}

int32 PWin32Platform::Swap()
{
    PEngine* Engine = PEngine::GetInstance();
    PRenderer* Renderer = Engine->GetRenderer();

    // Copy from the color channel into the display buffer
    void* Src = Renderer->GetColorChannel()->Memory;
    void* Dst = DisplayBuffer;
    size_t Size = Renderer->GetColorChannel()->GetMemorySize();
    memcpy(Dst, Src, Size);
    return 0;
}

FRect PWin32Platform::GetSize()
{
    RECT OutRect;

    if (GetWindowRect(GetHWnd(), &OutRect))
    {
        float Width = static_cast<float>(OutRect.right - OutRect.left);
        float Height = static_cast<float>(OutRect.bottom - OutRect.top);
        return {0, 0, Width, Height};
    }

    LOG_ERROR("Unable to get window size (PWin32Platform::GetSize).")
    return {};
}

bool PWin32Platform::GetFileDialog(std::string& OutFileName)
{
    OPENFILENAME OFN;
    TCHAR szFile[260] = {0};

    ZeroMemory(&OFN, sizeof(OFN));
    OFN.lStructSize = sizeof(OFN);
    OFN.hwndOwner = Hwnd;
    OFN.lpstrFile = szFile;
    OFN.nMaxFile = sizeof(szFile);
    OFN.lpstrFilter = static_cast<LPCWSTR>(L".obj\0*.obj\0");
    OFN.nFilterIndex = 1;
    OFN.lpstrFileTitle = nullptr;
    OFN.nMaxFileTitle = 0;
    OFN.lpstrInitialDir = nullptr;
    OFN.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (!GetOpenFileName(&OFN))
    {
        return false;
    }
    int32 FileSize = WideCharToMultiByte(CP_UTF8, 0, OFN.lpstrFile, -1, nullptr, 0, nullptr, nullptr);
    auto Tmp = new int8[FileSize];
    WideCharToMultiByte(CP_UTF8, 0, OFN.lpstrFile, -1, Tmp, FileSize, nullptr, nullptr);
    OutFileName = std::string(Tmp);
    return true;
}

void PWin32Platform::ConstructMenuBar()
{
    MainMenu = CreateMenu();
    FileMenu = CreateMenu();
	SettingsMenu = CreateMenu();

    // File menu
    AppendMenuW(MainMenu, MF_POPUP, UINT_PTR(FileMenu), L"&File");
    AppendMenuW(FileMenu, MF_STRING, UINT_PTR(EMenuAction::Open), L"&Open...");
    AppendMenuW(FileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(FileMenu, MF_STRING, UINT_PTR(EMenuAction::Quit), L"&Quit");

    // Display menu
    AppendMenuW(MainMenu, MF_POPUP, UINT_PTR(SettingsMenu), L"&Settings");
    AppendMenuW(SettingsMenu, MF_UNCHECKED, UINT_PTR(EMenuAction::Wireframe), L"&Wireframe");
    AppendMenuW(SettingsMenu, MF_CHECKED, UINT_PTR(EMenuAction::Shaded), L"&Shaded");
    AppendMenuW(SettingsMenu, MF_CHECKED, UINT_PTR(EMenuAction::Depth), L"&Depth");
    AppendMenuW(SettingsMenu, MF_UNCHECKED, UINT_PTR(EMenuAction::Normals), L"&Normals");

	AppendMenuW(SettingsMenu, MF_SEPARATOR, 0, L"");

	// Multithreading
	AppendMenuW(SettingsMenu, MF_CHECKED, UINT_PTR(EMenuAction::Multithreaded), L"&Multithreading");

    // Add the main menubar to the window
    SetMenu(Hwnd, MainMenu);
}

void PWin32Platform::SetMenuItemChecked(EMenuAction ActionId, const bool bChecked)
{
    CheckMenuItem(SettingsMenu, UINT_PTR(ActionId), bChecked ? MF_CHECKED : MF_UNCHECKED);
}
