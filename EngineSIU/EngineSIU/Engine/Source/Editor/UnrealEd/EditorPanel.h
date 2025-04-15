#pragma once

#ifndef __ICON_FONT_INDEX__

#define __ICON_FONT_INDEX__
#define DEFAULT_FONT		0
#define	FEATHER_FONT		1

#include "Math/Vector.h"
#include "SlateCore/Layout/SlateRect.h"

#endif // !__ICON_FONT_INDEX__


class UEditorPanel
{
public:
    virtual ~UEditorPanel() = default;
    virtual void Render() = 0;
    virtual void OnResize(HWND hWnd) = 0;


public:
    //void UpdateRect(FVector2D screenSize)
    //{
        // FVector2D AnchorMinPos = FMath::Lerp(FVector2D(0, 0), screenSize, AnchorMin);
        // FVector2D anchorMaxPos = FMath::Lerp(FVector2D(0, 0), screenSize, AnchorMax);
        //
        // FVector2D anchorAreaSize = anchorMaxPos - AnchorMinPos;
        // FVector2D Size = anchorAreaSize + SizeDelta;
        //
        // FVector2D PivotOffset = Size * Pivot;
        // FVector2D pos = AnchorMinPos + AnchoredPosition - PivotOffset;
        //
        // Rect.Min = pos;
        // Rect.Max = pos + Size;
    //}

     
    
    //void SetPivot(FVector2D InPivot) { Pivot = InPivot; }

protected:
    
    FSlateRect Rect = FSlateRect(0, 0, 0, 0);
    FSlateRect Padding = FSlateRect(0, 0, 0, 0);
    FVector2D AnchorMax = FVector2D(1, 1);
    FVector2D AnchorMin = FVector2D(0, 0);
    //FVector2D AnchorMin = FVector2D(0.5, 0.5);
    //FVector2D AnchorMax = FVector2D(0.5, 0.5);
    //FVector2D Pivot = FVector2D(0.5, 0.5);
    
    //FVector2D AnchoredPosition;
    // FVector2D SizeDelta;
    // FVector2D OffsetMin;
    // FVector2D OffsetMax;
};
