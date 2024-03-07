#pragma once
#include <map>
#include "Math/Vector.h"

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

struct FKey
{
    const char* Name;
};

class IInputHandler
{
public:
    static IInputHandler* GetInstance();

protected:
    bool bMouseLeftDown = false;
    bool bMouseRightDown = false;
    bool bMouseMiddleDown = false;

    FVector2 ClickPosition;
    FVector2 CurrentCursorPosition;
    FVector2 DeltaCursorPosition;

    std::map<const char, bool> KeyStateMap;

    IInputHandler()
    {
        for (uint8 Index = 32; Index < 127; Index++) // ASCII Space -> ~
        {
            KeyStateMap.emplace(Index, false);
        }
    }
    ~IInputHandler() = default;

public:
    // Mouse
    virtual bool OnMouseDown(EMouseButtonType ButtonType, const FVector2& CursorPosition) { return false; }
    virtual bool OnMouseUp(EMouseButtonType ButtonType, const FVector2& CursorPosition) { return false; }
    virtual bool OnMouseWheel(float Delta) { return false; }
    virtual bool OnMouseMove(FVector2 CursorPosition) { return false; }
    virtual bool IsMouseDown(EMouseButtonType ButtonType) const { return false; }
    virtual bool IsAnyMouseDown() const { return false; }

    virtual FVector2 GetCurrentCursorPosition() const { return CurrentCursorPosition; }
    virtual FVector2 GetDeltaCursorPosition() const { return DeltaCursorPosition; }

    // Keys
    virtual std::vector<char> GetKeysDown() const
    {
        std::vector<char> Keys;
        for (const auto& [K, V] : KeyStateMap)
        {
            if (V)
            {
                Keys.emplace_back(K);
            }
        }
        return Keys;
    }
    virtual bool OnKeyDown(int32 KeyCode, int32 KeyFlags, bool bIsRepeat) { return false; }
    virtual bool OnKeyUp(int32 KeyCode, int32 KeyFlags, bool bIsRepeat) { return false; }
    virtual bool IsKeyDown(int32 KeyCode) const { return false; }
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
            break;
        case EMouseButtonType::Right :
            bMouseRightDown = true;
            break;
        case EMouseButtonType::Middle :
            bMouseMiddleDown = true;
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
            break;
        case EMouseButtonType::Right :
            bMouseRightDown = false;
            break;
        case EMouseButtonType::Middle :
            bMouseMiddleDown = false;
            break;
        case EMouseButtonType::Invalid :
        default :
            return false;
        }

        return true;
    }

    bool OnMouseWheel(float Delta) override
    {
        return true;
    }

    bool OnMouseMove(FVector2 CursorPosition) override
    {
        // Update current cursor position
        CurrentCursorPosition = CursorPosition;

        // Update delta cursor position if any of the mouse buttons are down
        if (IsAnyMouseDown())
        {
            DeltaCursorPosition.X = CursorPosition.X - ClickPosition.X;
            DeltaCursorPosition.Y = CursorPosition.Y - ClickPosition.Y;
        }
        // If no mouse buttons are down, zero the delta cursor position
        else
        {
            DeltaCursorPosition.X = 0.0f;
            DeltaCursorPosition.Y = 0.0f;
        }

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

    bool OnKeyDown(int32 KeyCode, int32 KeyFlags, bool bIsRepeat) override
    {
        const char Char = static_cast<char>(KeyCode);
        KeyStateMap[Char] = true;
        return true;
    }

    bool OnKeyUp(int32 KeyCode, int32 KeyFlags, bool bIsRepeat) override
    {
        const char Char = static_cast<char>(KeyCode);
        KeyStateMap[Char] = false;
        return true;
    }

    bool IsKeyDown(int32 KeyCode) const override
    {
        const char Char = static_cast<char>(KeyCode);
        return KeyStateMap.at(Char);
    }
};
