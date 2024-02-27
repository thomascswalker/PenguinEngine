#include <Framework/Platforms/WindowsPlatform.h>

#include "Framework/Core/Core.h"

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
            constexpr COLORREF Color = COLOR_DESKTOP;
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
    default:
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

void PWindowsPlatform::Create()
{
    Register();
    bInitialized = HWnd != nullptr;
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWindowsPlatform::Create).")
    }
}

int PWindowsPlatform::Show()
{
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWindowsPlatform::Show).")
        return 3; // Show window failure
    }

    int Result = ShowWindow(HWnd, 1);
    LOG_INFO("Showing window.")
    return Result;
}

int PWindowsPlatform::Loop()
{
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWindowsPlatform::Loop).")
        return 4; // Loop failure
    }

    LOG_INFO("Beginning window loop.")
    
    MSG Msg = {};
    while (GetMessage(&Msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    LOG_INFO("Ending window loop.")

    return 0;
}
