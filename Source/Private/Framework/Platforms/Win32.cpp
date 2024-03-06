#include <cassert>

#include "Framework/Platforms/Win32Platform.h"
#include "Framework/Core/ErrorCodes.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Engine/Timer.h"

#ifndef WINDOWS_TIMER_ID
    #define WINDOWS_TIMER_ID 1001
#endif

const float MovementSpeed = 0.05f;
bool bShowText = true;

LRESULT PWin32Platform::WindowProc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    PEngine* Engine = PEngine::GetInstance();
    const PRenderer* Renderer = Engine->GetRenderer();

    WORD KeyCode;
    WORD KeyFlags;

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
            Engine->Shutdown();
            PostQuitMessage(0);
            return 0;
        }
    case WM_KEYDOWN :
    case WM_SYSKEYDOWN :
        {
            KeyCode = LOWORD(WParam);
            KeyFlags = HIWORD(LParam);

            switch (KeyCode)
            {
            // Forward
            case 'W' :
                Renderer->GetViewport()->AddViewTranslation(FVector3::ForwardVector() * MovementSpeed);
                break;
            // Backward
            case 'S' :
                Renderer->GetViewport()->AddViewTranslation(FVector3::ForwardVector() * -MovementSpeed);
                break;
            // Right
            case 'D' :
                Renderer->GetViewport()->AddViewTranslation(FVector3::RightVector() * MovementSpeed);
                break;
            // Left
            case 'A' :
                Renderer->GetViewport()->AddViewTranslation(FVector3::RightVector() * -MovementSpeed);
                break;
            // Up
            case 'Q' :
                Renderer->GetViewport()->AddViewTranslation(FVector3::UpVector() * MovementSpeed);
                break;
            // Down
            case 'E' :
                Renderer->GetViewport()->AddViewTranslation(FVector3::UpVector() * -MovementSpeed);
                break;
            case 'T':
                bShowText = !bShowText;
                break;
            case VK_ESCAPE :
                Engine->SetRunning(false);
                break;
            default :
                break;
            }
            break;
        }
    case WM_PAINT :
        {
            if (!Renderer)
            {
                LOG_WARNING("Renderer is not initialized in AppWindowProc::WM_PAINT")
                break;
            }

            // Get the current window size from the buffer
            const std::shared_ptr<PBuffer> Buffer = Renderer->GetColorBuffer();
            const uint32 Width = Buffer->Width;
            const uint32 Height = Buffer->Height;

            // Create a bitmap with the current renderer buffer memory the size of the window
            InvalidateRect(Hwnd, nullptr, TRUE);
            PAINTSTRUCT Paint;
            const HDC DeviceContext = BeginPaint(Hwnd, &Paint);
            const HDC RenderContext = CreateCompatibleDC(DeviceContext);
            const HBITMAP Bitmap = CreateBitmap(Width, Height, 1, 32, Buffer->Memory); // NOLINT
            SelectObject(RenderContext, Bitmap);
            if (!BitBlt(DeviceContext, 0, 0, Width, Height, RenderContext, 0, 0, SRCCOPY)) // NOLINT
            {
                LOG_ERROR("Failed during BitBlt")
                Result = 1;
            }

            auto ViewString = Renderer->GetViewport()->GetInfo()->ViewRotationMatrix.ToString();
            auto ProjString = Renderer->GetViewport()->GetInfo()->ProjectionMatrix.ToString();
            auto MvpString = Renderer->GetViewport()->GetViewProjectionMatrix()->ToString();

            if (bShowText)
            {
                // Draw text indicating the current FPS
                RECT ClientRect;
                GetClientRect(Hwnd, &ClientRect);
                ClientRect.top += 10;
                ClientRect.left += 10;

                std::string FpsMessage = std::to_string(static_cast<uint32>(Engine->GetFps()));
                std::string OutputMessage = std::format("FPS: {}\nResolution: [{}, {}]\nView: {}\nProjection: {}\nMVP: {}", FpsMessage, Width, Height, ViewString, ProjString, MvpString);
                SetTextColor(DeviceContext, RGB(0, 255, 0));
                SetBkColor(DeviceContext, TRANSPARENT);
                DrawText(
                    DeviceContext, // DC
                    std::wstring(OutputMessage.begin(), OutputMessage.end()).c_str(), // Message
                    -1,
                    &ClientRect, // Client rectangle (the window)
                    DT_TOP | DT_LEFT // Drawing options
                );
            }

            // Cleanup and end painting
            ReleaseDC(Hwnd, DeviceContext);
            DeleteDC(DeviceContext);
            DeleteDC(RenderContext);
            DeleteObject(Bitmap);
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
            const int Width = LOWORD(LParam);
            const int Height = HIWORD(LParam);

            // Update the renderer size
            Renderer->Resize(Width, Height);
            break;
        }
    case WM_EXITSIZEMOVE :
    case WM_ERASEBKGND :
        {
            InvalidateRect(Hwnd, nullptr, TRUE);
            break;
        }
    // Timer called every ms to update
    case WM_TIMER :
        {
            InvalidateRect(Hwnd, nullptr, FALSE);
            UpdateWindow(Hwnd);
            break;
        }
    default :
        {
            Result = DefWindowProcW(Hwnd, Msg, WParam, LParam);
            break;
        }
    }

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

uint32 PWin32Platform::Create()
{
    bInitialized = Register();
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWin32Platform::Create).")
        return PlatformInitError;
    }
    return Success;
}

uint32 PWin32Platform::Show()
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

uint32 PWin32Platform::Start()
{
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWin32Platform::Start).")
        return PlatformStartError; // Start failure
    }

    // Start the actual engine
    PEngine* Engine = PEngine::GetInstance();

    // Initialize the engine
    RECT ClientRect;
    GetClientRect(Hwnd, &ClientRect);
    Engine->Startup(ClientRect.right, ClientRect.bottom);

    // Process all messages and update the window
    LOG_DEBUG("Engine loop start")
    MSG Msg = {};
    while (Engine->IsRunning() && GetMessage(&Msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);

        // Loop, converting DeltaTime from milliseconds to seconds
        if (const uint32 LoopResult = Loop())
        {
            LOG_ERROR("Loop failed (PWin32Platform::Start).")
            return LoopResult; // Start failure
        }
    }
    LOG_INFO("Ending engine loop.")

    return End();
}

uint32 PWin32Platform::Loop()
{
    // Tick the engine forward
    if (PEngine* Engine = PEngine::GetInstance())
    {
        Engine->Tick();

        // Render the frame
        if (const PRenderer* Renderer = Engine->GetRenderer())
        {
            Renderer->Render();
            return Success;
        }
    }
    return PlatformLoopError;
}

uint32 PWin32Platform::Paint()
{
    return 0;
}

uint32 PWin32Platform::End()
{
    return Success;
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
