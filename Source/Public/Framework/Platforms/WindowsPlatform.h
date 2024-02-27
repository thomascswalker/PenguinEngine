#pragma once

#include <windows.h>

#include "Framework/Core/Core.h"
#include "Framework/Platform.h"

class PWindowsPlatform : public IPlatform
{
    LPCWSTR ClassName = L"PenguinWindow";
    LPCWSTR WindowName = L"Penguin Renderer";
    HWND HWnd = nullptr;
    DWORD Style = WS_OVERLAPPEDWINDOW;
    int X = CW_USEDEFAULT;
    int Y = CW_USEDEFAULT;
    int Width = CW_USEDEFAULT;
    int Height = CW_USEDEFAULT;
    
    bool bInitialized = false;

    HINSTANCE HInstance;
    void Register();

public:
    // Platform interface
    int Create() override;
    int Show() override;
    int Start() override;
    int Loop(float DeltaTime) override;
    int End() override;
    bool IsInitialized() const override { return bInitialized; }

    // Windows
    explicit PWindowsPlatform(HINSTANCE NewInstance) : HInstance(NewInstance) {}
    HWND GetHWnd() const { return HWnd; }
    void SetHInstance(HINSTANCE NewInstance) { HInstance = NewInstance; }
};
