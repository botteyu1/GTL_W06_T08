#pragma once

#include "Container/Set.h"
#include "SlateCore/Layout/SlateRect.h"

class SWindow
{
public:
    SWindow() = default;
    SWindow(FSlateRect initRect);
    virtual ~SWindow() = default;

    virtual void Initialize(SWindow* InParent)
    {
        this->Parent = InParent;
    }
    
    virtual void Release();

    virtual void LoadConfig(const TMap<FString, FString>& config);
    virtual void SaveConfig(TMap<FString, FString>& config) const;

public:
    virtual void OnResize(double DeltaWidthRatio, double DeltaHeightRatio);
public:
    void AddChildren(SWindow* Child)
    {
        Children.Add(Child);
        Child->Parent = this;
    }
    
public:
    void SetRect(FSlateRect newRect) { Rect = newRect; }
    bool IsHover(FVector2D coord) const;
    bool IsHover(FPoint coord) const;
    virtual void OnPressed(FPoint coord);
    virtual void OnReleased();
    bool IsPressing() const { return bIsPressed; }

public:
    FSlateRect Rect = FSlateRect(0, 0, 0, 0);
    
protected:
    bool bIsHoverd = false;
    bool bIsPressed = false;

protected:
    SWindow* Parent = nullptr;
    TSet<SWindow*> Children;
};

