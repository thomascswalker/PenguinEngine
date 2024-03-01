#include <cassert>
#include <Framework/Platforms/WindowsPlatform.h>

#include "Framework/Application.h"
#include "Framework/Core/ErrorCodes.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Engine/Timer.h"

#ifndef WINDOWS_TIMER_ID
    #define WINDOWS_TIMER_ID 1001
#endif

// ReSharper disable CppParameterMayBeConst
LRESULT CALLBACK AppWindowProc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    PEngine* Engine = PEngine::GetInstance();
    const PRenderer* Renderer = Engine->GetRenderer();

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
    case WM_PAINT :
        {
            if (!Renderer)
            {
                LOG_WARNING("Renderer is not initialized in AppWindowProc::WM_PAINT")
                break;
            }

            // Get the current window size from the buffer
            const PBuffer* Buffer = Renderer->GetBuffer();
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

            // Draw text indicating the current FPS
            RECT ClientRect;
            GetClientRect(Hwnd, &ClientRect);
            ClientRect.top += 10;
            ClientRect.right -= 10;

            std::string FpsMessage = std::to_string(static_cast<uint32>(Engine->GetFps()));
            SetTextColor(DeviceContext, RGB(0, 255, 0));
            SetBkColor(DeviceContext, TRANSPARENT);
            DrawText(
                DeviceContext, // DC
                std::wstring(FpsMessage.begin(), FpsMessage.end()).c_str(), // Message
                -1,
                &ClientRect, // Client rectangle (the window)
                DT_TOP | DT_RIGHT // Drawing options
            );

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

bool PWindowsPlatform::Register()
{
    // Register the window class.
    WNDCLASS WindowClass = {};

    WindowClass.lpfnWndProc = AppWindowProc;
    WindowClass.hInstance = HInstance;
    WindowClass.lpszClassName = ClassName;

    //Registering the window class
    if (!RegisterClass(&WindowClass))
    {
        LOG_ERROR("Failed to register class (PWindowsPlatform::Register).")
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
        LOG_ERROR("Failed to create window (PWindowsPlatform::Register).")
        return false;
    }

    LOG_INFO("Registered class.")
    return true;
}

uint32 PWindowsPlatform::Create()
{
    bInitialized = Register();
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWindowsPlatform::Create).")
        return PlatformInitError;
    }
    return Success;
}

uint32 PWindowsPlatform::Show()
{
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWindowsPlatform::Show).")
        return PlatformShowError; // Show window failure
    }

    LOG_INFO("Showing window.")
    ShowWindow(Hwnd, 1);
    return Success;
}

uint32 PWindowsPlatform::Start()
{
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWindowsPlatform::Start).")
        return PlatformStartError; // Start failure
    }

    // Start the actual engine
    PEngine* Engine = PEngine::GetInstance();

    // Initialize the engine
    RECT ClientRect;
    GetClientRect(Hwnd, &ClientRect);
    Engine->Startup(ClientRect.right, ClientRect.bottom);
    // InvalidateRect(Hwnd, nullptr, FALSE);

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
            LOG_ERROR("Loop failed (PWindowsPlatform::Start).")
            return LoopResult; // Start failure
        }
    }
    LOG_INFO("Ending engine loop.")

    return End();
}

uint32 PWindowsPlatform::Loop()
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

uint32 PWindowsPlatform::Paint()
{
    return 0;
}

uint32 PWindowsPlatform::End()
{
    return Success;
}

PRectI PWindowsPlatform::GetSize()
{
    RECT OutRect;

    if (GetWindowRect(GetHWnd(), &OutRect))
    {
        int Width = OutRect.right - OutRect.left;
        int Height = OutRect.bottom - OutRect.top;
        return {0, 0, Width, Height};
    }

    LOG_ERROR("Unable to get window size (PWindowsPlatform::GetSize).")
    return {};
}
