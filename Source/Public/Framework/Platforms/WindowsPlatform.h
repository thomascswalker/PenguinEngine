#pragma once

#include <windows.h>

#include "Framework/Core/Core.h"
#include "Framework/Platform.h"

class PWindowsPlatform : public IPlatform
{
    // Windows specific variables
    LPCWSTR ClassName = L"PenguinWindow";
    LPCWSTR WindowName = L"Penguin Renderer";
    HWND Hwnd = nullptr;
    DWORD DefaultStyle = WS_OVERLAPPEDWINDOW;
    int DefaultX = CW_USEDEFAULT;
    int DefaultY = CW_USEDEFAULT;
    int DefaultWidth = CW_USEDEFAULT;
    int DefaultHeight = CW_USEDEFAULT;
    BITMAPINFO BitmapInfo;

    bool bInitialized = false;

    HINSTANCE HInstance;
    bool Register();

public:
    // Platform interface
    int Create() override;
    int Show() override;
    int Start() override;
    int Loop(float DeltaTime) override;
    int End() override;
    bool IsInitialized() const override { return bInitialized; }

    // Windows
    PWindowsPlatform(HINSTANCE NewInstance) : HInstance(NewInstance)
    {
        InitBitmapInfo();
    }
    HWND GetHWnd() const { return Hwnd; }
    void SetHInstance(HINSTANCE NewInstance) { HInstance = NewInstance; }
    RectI GetSize() override;

    void InitBitmapInfo()
    {
        BitmapInfo.bmiHeader.biSize = sizeof(GetBitmapInfo()->bmiHeader);
        BitmapInfo.bmiHeader.biWidth = DefaultWidth;
        BitmapInfo.bmiHeader.biHeight = -DefaultHeight; // Otherwise Y is inverted
        BitmapInfo.bmiHeader.biPlanes = 1;
        BitmapInfo.bmiHeader.biBitCount = 32;
        BitmapInfo.bmiHeader.biCompression = BI_RGB;
    }
    BITMAPINFO* GetBitmapInfo()
    {
        UpdateBitmapInfo();
        return &BitmapInfo;
    }
    void UpdateBitmapInfo()
    {
        const RectI Size = GetSize();
        BitmapInfo.bmiHeader.biWidth = Size.Width;
        BitmapInfo.bmiHeader.biHeight = Size.Height;
    }
};
