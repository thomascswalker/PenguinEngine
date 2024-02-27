#include <Framework/Platforms/WindowsPlatform.h>

#include "Framework/Core/ErrorCodes.h"
#include "Framework/Engine/Engine.h"

// ReSharper disable CppParameterMayBeConst
LRESULT CALLBACK AppWindowProc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam)
{
    switch (Msg)
    {
    case WM_DESTROY :
        {
            PostQuitMessage(0);
            return 0;
        }
    case WM_PAINT :
        {
            PAINTSTRUCT PS;
            const HDC HDC = BeginPaint(Hwnd, &PS);

            // All painting occurs here, between BeginPaint and EndPaint.
            constexpr COLORREF Color = COLOR_3DLIGHT;
            const HBRUSH Brush = CreateSolidBrush(Color);
            FillRect(HDC, &PS.rcPaint, Brush);

            EndPaint(Hwnd, &PS);

            return 0;
        }
    case WM_SIZE :
        {
            // LOG_DEBUG("Resizing: {}, {}", LOWORD(LParam), HIWORD(LParam))
            return 0;
        }
    default :
        break;
    }
    return DefWindowProc(Hwnd, Msg, WParam, LParam);
}

void PWindowsPlatform::Register()
{
    // Register the window class.
    WNDCLASS WindowClass = {};

    WindowClass.lpfnWndProc = AppWindowProc;
    WindowClass.hInstance = HInstance;
    WindowClass.lpszClassName = ClassName;

    RegisterClassW(&WindowClass);

    // Create the window.
    HWnd = CreateWindowExW(
        0,
        ClassName,
        WindowName,
        Style,
        X, Y, Width, Height,
        nullptr,
        nullptr,
        HInstance,
        nullptr
    );

    LOG_INFO("Registered class.")
}

int PWindowsPlatform::Create()
{
    Register();
    bInitialized = HWnd != nullptr;
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWindowsPlatform::Create).")
        return PlatformInitError;
    }
    return Success;
}

int PWindowsPlatform::Show()
{
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWindowsPlatform::Show).")
        return PlatformShowError; // Show window failure
    }

    LOG_INFO("Showing window.")
    ShowWindow(HWnd, 1);
    return Success;
}

int PWindowsPlatform::Start()
{
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWindowsPlatform::Start).")
        return PlatformStartError; // Start failure
    }

    MSG Msg = {};
    clock_t Time = clock();

    // Start the actual engine
    PEngine::GetInstance()->Startup();

    // Windows loop
    LOG_INFO("Beginning window loop.")
    while (GetMessage(&Msg, nullptr, 0, 0) > 0)
    {
        // Get delta time in milliseconds
        const float DeltaTime = static_cast<float>(clock() - Time);

        // Loop, converting DeltaTime from milliseconds to seconds
        if (const int LoopResult = Loop(1.0f / DeltaTime))
        {
            LOG_ERROR("Loop failed (PWindowsPlatform::Start).")
            return LoopResult; // Start failure
        }
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
        Time = clock();
    }
    LOG_INFO("Ending window loop.")

    return End();
}

int PWindowsPlatform::Loop(float DeltaTime)
{
    PEngine::GetInstance()->Tick(DeltaTime);
    return Success;
}

int PWindowsPlatform::End()
{
    if (!PEngine::GetInstance()->Shutdown())
    {
        return PlatformEndError;
    }
    return Success;
}
