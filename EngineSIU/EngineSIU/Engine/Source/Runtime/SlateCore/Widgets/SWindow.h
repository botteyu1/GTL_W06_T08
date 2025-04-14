#pragma once

#include "SlateCore/Layout/SlateRect.h"

class SWindow
{
public:
    SWindow() = default;
    SWindow(FSlateRect initRect);
    virtual ~SWindow() = default;

    virtual void Initialize(FSlateRect initRect);
    virtual void OnResize(float width, float height);

    void SetRect(FSlateRect newRect) { Rect = newRect; }
    bool IsHover(FPoint coord) const;
    virtual bool OnPressed(FPoint coord);
    virtual bool OnReleased();
    bool IsPressing() const { return bIsPressed; }

public:
    FSlateRect Rect = FSlateRect(0, 0, 0, 0);
    
protected:
    bool bIsHoverd = false;
    bool bIsPressed = false;

};

