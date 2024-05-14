#pragma once

#include <windows.h>

#include "PlatformInterface.h"
#include "Framework/Renderer/Viewport.h"

class PWin32Platform : public IPlatform
{
    // Windows specific variables
    LPCWSTR ClassName = L"PenguinWindow";
    LPCWSTR WindowName = L"Penguin Renderer";
    HWND Hwnd = nullptr;
    DWORD DefaultStyle = WS_OVERLAPPEDWINDOW;
    int32 DefaultX = CW_USEDEFAULT;
    int32 DefaultY = CW_USEDEFAULT;
    int32 DefaultWidth = DEFAULT_VIEWPORT_WIDTH;
    int32 DefaultHeight = DEFAULT_VIEWPORT_HEIGHT;

    inline static BITMAPINFO BitmapInfo;
    inline static HBITMAP DisplayBitmap;
    inline static int8* DisplayBuffer;

    bool bInitialized = false;

    HINSTANCE HInstance;
    bool Register();

public:
    // Platform interface
    uint32 Create() override;
    uint32 Show() override;
    uint32 Start() override;
    uint32 Loop() override;
    uint32 Paint() override;
    uint32 End() override;
    uint32 Swap() override;
    bool IsInitialized() const override { return bInitialized; }

    // Windows
    PWin32Platform(HINSTANCE NewInstance) : HInstance(NewInstance)
    {
    }
    static LRESULT CALLBACK WindowProc(HWND Hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    HWND GetHWnd() const { return Hwnd; }
    void SetHInstance(HINSTANCE NewInstance) { HInstance = NewInstance; }
    FRect GetSize() override;
    EPlatformType GetPlatformType() override { return EPlatformType::Windows; }
    // FRect GetSize() override;
};
