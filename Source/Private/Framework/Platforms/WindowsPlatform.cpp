#include <assert.h>
#include <Framework/Platforms/WindowsPlatform.h>

#include "Framework/Application.h"
#include "Framework/Core/ErrorCodes.h"
#include "Framework/Engine/Engine.h"

#ifndef WINDOWS_TIMER_ID
    #define WINDOWS_TIMER_ID 1001
#endif


// ReSharper disable CppParameterMayBeConst
LRESULT CALLBACK AppWindowProc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    PWindowsPlatform* Platform = Cast<PWindowsPlatform>(PApplication::GetInstance()->GetPlatform());
    PEngine* Engine = PEngine::GetInstance();
    std::shared_ptr<PRenderer> Renderer = Engine->GetRenderer();

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
            PEngine* Engine = PEngine::GetInstance();
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
            InvalidateRect(Hwnd, nullptr, TRUE);
            PAINTSTRUCT Paint;
            const HDC DeviceContext = BeginPaint(Hwnd, &Paint);

            HDC RenderContext = CreateCompatibleDC(DeviceContext);
            PBuffer* Buffer = Renderer->GetBuffer();
            int Width = Buffer->Width;
            int Height = Buffer->Height;

            HBITMAP Bitmap = CreateBitmap(Width, Height, 1, 32, Buffer->Memory);
            SelectObject(RenderContext, Bitmap);
            if (!BitBlt(DeviceContext, 0, 0, Width, Height, RenderContext, 0, 0, SRCCOPY))
            {
                LOG_ERROR("Failed during BitBlt")
                Result = 1;
            }
            ReleaseDC(Hwnd, DeviceContext);
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
            Renderer->SetSize(Width, Height);

            // Re-allocate the buffer memory
            Renderer->Realloc();

            // Update the bitmap info
            Platform->UpdateBitmapInfo();
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

int PWindowsPlatform::Create()
{
    bInitialized = Register();
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
    ShowWindow(Hwnd, 1);
    return Success;
}

int PWindowsPlatform::Start()
{
    if (!bInitialized)
    {
        LOG_ERROR("Window failed to initialize (PWindowsPlatform::Start).")
        return PlatformStartError; // Start failure
    }

    // Start a clock to track DeltaTime
    clock_t Time = clock();

    // Start the actual engine
    PEngine* Engine = PEngine::GetInstance();
    
    // Initialize the engine
    RECT ClientRect;
    GetWindowRect(Hwnd, &ClientRect);
    Engine->Startup(ClientRect.right, ClientRect.bottom);

    // Windows loop
    LOG_INFO("Beginning engine loop.")
    while (Engine->IsRunning())
    {
        LOG_DEBUG("Engine loop start")

        InvalidateRect(Hwnd, nullptr, FALSE);

        // Process all messages and update the window
        MSG Msg = {};
        while (GetMessage(&Msg, nullptr, 0, 0) > 0)
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);

            // Get delta time in milliseconds
            const float DeltaTime = static_cast<float>(clock() - Time);

            // Loop, converting DeltaTime from milliseconds to seconds
            if (const int LoopResult = Loop(1.0f / DeltaTime))
            {
                LOG_ERROR("Loop failed (PWindowsPlatform::Start).")
                return LoopResult; // Start failure
            }
            Time = clock();
        }

        UpdateWindow(Hwnd);
    }
    LOG_INFO("Ending engine loop.")

    return End();
}

int PWindowsPlatform::Loop(float DeltaTime)
{
    // Tick the engine forward
    PEngine* Engine = PEngine::GetInstance();
    Engine->Tick(DeltaTime);

    // Render the frame
    Engine->GetRenderer()->Render();

    return Success;
}

int PWindowsPlatform::End()
{
    return Success;
}

RectI PWindowsPlatform::GetSize()
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
