#pragma once

#include <map>

#include "Framework/Core/Bitmask.h"
#include "Math/Vector.h"
#include "Framework/Engine/Delegate.h"

class IInputHandler;
class PWin32InputHandler;
class PMacOSInputHandler;
class PLinuxInputHandler;

enum class EMouseButtonType : uint8
{
    Invalid,
    Left,
    Right,
    Middle
};

enum class EKey : uint8
{
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Zero,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    Escape,
    Spacebar,
    Enter,
    Ctrl,
    Shift,
    Alt,
    CapsLock,
    Backspace,

    // Meta
    Count
};

enum class EModifierKey : uint8
{
    None  = 0x00,
    Shift = 0x01,
    Ctrl  = 0x02,
    Alt   = 0x04
};
DEFINE_BITMASK_OPERATORS(EModifierKey)

struct FKey
{
    const char* Name;
};

enum class EMenuAction : uint16
{
    // File
    Open,
    Quit,

    // Display
    Wireframe,
    Shaded,
    Depth,
    Normals,
    VertexNormals,

	// Settings
	Multithreaded
};

DECLARE_MULTICAST_DELEGATE(FOnMouseMoved, const FVector2&);
DECLARE_MULTICAST_DELEGATE(FOnMouseLeftDown, const FVector2&);
DECLARE_MULTICAST_DELEGATE(FOnMouseLeftUp, const FVector2&);
DECLARE_MULTICAST_DELEGATE(FOnMouseRightDown, const FVector2&);
DECLARE_MULTICAST_DELEGATE(FOnMouseRightUp, const FVector2&);
DECLARE_MULTICAST_DELEGATE(FOnMouseMiddleDown, const FVector2&);
DECLARE_MULTICAST_DELEGATE(FOnMouseMiddleUp, const FVector2&);

DECLARE_MULTICAST_DELEGATE(FOnKeyPressed, EKey);
DECLARE_MULTICAST_DELEGATE(FOnMouseMiddleScrolled, float);

DECLARE_MULTICAST_DELEGATE(FOnMenuActionPressed, EMenuAction);

class IInputHandler
{
public:
    static IInputHandler* GetInstance();

    // Events

protected:
    bool bMouseLeftDown = false;
    bool bMouseRightDown = false;
    bool bMouseMiddleDown = false;

    FVector2 ClickPosition;
    FVector2 CurrentCursorPosition;
    FVector2 PreviousCursorPosition;
    FVector2 DeltaCursorPosition;

    std::map<EKey, bool> KeyStateMap;
    EModifierKey ModifierKeys = EModifierKey::None;

    IInputHandler()
    {
        const uint8 KeyCount = static_cast<uint8>(EKey::Count);
        for (uint8 Index = 0; Index < KeyCount; Index++)
        {
            KeyStateMap.emplace(static_cast<EKey>(Index), false);
        }
    }
    ~IInputHandler() = default;

public:
    // Events
    FOnMouseMoved MouseMoved;
    FOnMouseLeftDown MouseLeftDown;
    FOnMouseRightDown MouseRightDown;
    FOnMouseMiddleDown MouseMiddleDown;
    FOnMouseLeftUp MouseLeftUp;
    FOnMouseRightUp MouseRightUp;
    FOnMouseMiddleUp MouseMiddleUp;
    FOnMouseMiddleScrolled MouseMiddleScrolled;
    FOnKeyPressed KeyPressed;
    FOnMenuActionPressed MenuActionPressed;

    // Mouse
    virtual bool OnMouseDown(EMouseButtonType ButtonType, const FVector2& CursorPosition) { return false; }
    virtual bool OnMouseUp(EMouseButtonType ButtonType, const FVector2& CursorPosition) { return false; }
    virtual bool OnMouseWheel(float Delta) { return false; }
    virtual bool OnMouseMove(const FVector2& CursorPosition) { return false; }
    virtual bool IsMouseDown(EMouseButtonType ButtonType) const { return false; }
    virtual bool IsAnyMouseDown() const { return false; }

    virtual FVector2 GetClickPosition() const { return ClickPosition; }
    virtual FVector2 GetCurrentCursorPosition() const { return CurrentCursorPosition; }
    virtual FVector2 GetPreviousCursorPosition() const { return PreviousCursorPosition; }
    virtual FVector2 GetDeltaCursorPosition() const { return DeltaCursorPosition; }
    virtual void ResetDeltaCursorPosition()
    {
        DeltaCursorPosition.X = 0.0f;
        DeltaCursorPosition.Y = 0.0f;
    }

    // Keys
    virtual std::vector<EKey> GetKeysDown() const
    {
        std::vector<EKey> Keys;
        for (const auto& [K, V] : KeyStateMap)
        {
            if (V)
            {
                Keys.emplace_back(K);
            }
        }
        return Keys;
    }
    virtual bool OnKeyDown(EKey KeyCode, int32 KeyFlags, bool bIsRepeat) { return false; }
    virtual bool OnKeyUp(EKey KeyCode, int32 KeyFlags, bool bIsRepeat) { return false; }
    virtual bool IsKeyDown(EKey KeyCode) const { return false; }
    virtual void ConsumeKey(const EKey KeyCode)
    {
        KeyStateMap.at(KeyCode) = false;
    }
    virtual bool IsAltDown() const
    {
        return KeyStateMap.at(EKey::Alt);
    }
    virtual bool IsShiftDown() const
    {
        return KeyStateMap.at(EKey::Shift);
    }
    virtual bool IsCtrlDown() const
    {
        return KeyStateMap.at(EKey::Ctrl);
    }

    virtual void OnMenuActionPressed(EMenuAction ActionId) = 0;
};

class PWin32InputHandler : public IInputHandler
{
protected:
    static PWin32InputHandler* Instance;
    PWin32InputHandler() = default;
    ~PWin32InputHandler() = default;

public:
    static PWin32InputHandler* GetInstance();

    bool OnMouseDown(EMouseButtonType ButtonType, const FVector2& CursorPosition) override
    {
        switch (ButtonType)
        {
        case EMouseButtonType::Left :
            bMouseLeftDown = true;
            MouseLeftDown.Broadcast(CursorPosition);
            break;
        case EMouseButtonType::Right :
            bMouseRightDown = true;
            MouseRightDown.Broadcast(CursorPosition);
            break;
        case EMouseButtonType::Middle :
            bMouseMiddleDown = true;
            MouseMiddleDown.Broadcast(CursorPosition);
            break;
        case EMouseButtonType::Invalid :
        default :
            return false;
        }

        ClickPosition = CursorPosition;

        return true;
    }
    bool OnMouseUp(EMouseButtonType ButtonType, const FVector2& CursorPosition) override
    {
        switch (ButtonType)
        {
        case EMouseButtonType::Left :
            bMouseLeftDown = false;
            MouseLeftUp.Broadcast(CursorPosition);
            break;
        case EMouseButtonType::Right :
            bMouseRightDown = false;
            MouseRightUp.Broadcast(CursorPosition);
            break;
        case EMouseButtonType::Middle :
            bMouseMiddleDown = false;
            MouseMiddleUp.Broadcast(CursorPosition);
            break;
        case EMouseButtonType::Invalid :
        default :
            return false;
        }
        ClickPosition = 0;

        return true;
    }

    bool OnMouseWheel(float Delta) override
    {
        // Invert delta
        MouseMiddleScrolled.Broadcast(Delta);
        return true;
    }

    bool OnMouseMove(const FVector2& CursorPosition) override
    {
        // Update current cursor position
        if (CurrentCursorPosition == CursorPosition)
        {
            DeltaCursorPosition = 0;
            return false;
        }
        PreviousCursorPosition = CurrentCursorPosition;
        CurrentCursorPosition = CursorPosition;
        MouseMoved.Broadcast(CurrentCursorPosition);
        DeltaCursorPosition = CurrentCursorPosition - PreviousCursorPosition;
        return true;
    }

    bool IsMouseDown(EMouseButtonType ButtonType) const override
    {
        switch (ButtonType)
        {
        case EMouseButtonType::Left :
            return bMouseLeftDown;
        case EMouseButtonType::Right :
            return bMouseRightDown;
        case EMouseButtonType::Middle :
            return bMouseMiddleDown;
        case EMouseButtonType::Invalid :
        default :
            return false;
        }
    }

    bool IsAnyMouseDown() const override
    {
        return bMouseLeftDown || bMouseRightDown || bMouseMiddleDown;
    }

    bool OnKeyDown(EKey KeyCode, int32 KeyFlags, bool bIsRepeat) override
    {
        KeyStateMap[KeyCode] = true;
        return true;
    }

    bool OnKeyUp(EKey KeyCode, int32 KeyFlags, bool bIsRepeat) override
    {
        KeyStateMap[KeyCode] = false;
        KeyPressed.Broadcast(KeyCode);
        return true;
    }

    bool IsKeyDown(EKey KeyCode) const override
    {
        return KeyStateMap.at(KeyCode);
    }

    void OnMenuActionPressed(EMenuAction ActionId) override
    {
        switch (ActionId)
        {
        case EMenuAction::Open :

            break;
        case EMenuAction::Quit :
            break;
        }
    }
};
