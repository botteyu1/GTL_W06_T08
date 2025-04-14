#include "SWindow.h"


SWindow::SWindow(FSlateRect initRect) : Rect(initRect)
{
}

void SWindow::Initialize(FSlateRect initRect)
{
    Rect = initRect;
}

void SWindow::OnResize(float width, float height)
{
    Rect.SetWidth(width);
    Rect.SetHeight(height);
}

bool SWindow::IsHover(FPoint coord) const
{
    return Rect.Contains(coord);
}

bool SWindow::OnPressed(FPoint coord)
{
    return false;
}

bool SWindow::OnReleased() 
{
    return false;
}
