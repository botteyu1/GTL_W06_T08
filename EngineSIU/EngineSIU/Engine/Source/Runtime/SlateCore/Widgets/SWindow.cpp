#include "SWindow.h"


SWindow::SWindow(FSlateRect initRect) : Rect(initRect)
{
}

void SWindow::OnResize(double DeltaWidthRatio, double DeltaHeightRatio)
{
    Rect.Left *= DeltaWidthRatio;
    Rect.Right *= DeltaWidthRatio;

    Rect.Top *= DeltaHeightRatio;
    Rect.Bottom *= DeltaHeightRatio;

    for (const auto& Child : Children)
    {
        Child->OnResize(DeltaWidthRatio, DeltaHeightRatio);
    } 
}

void SWindow::Release()
{
    // TODO Check - 안전한가?
    auto ChildrenCopy = Children;
    for (const auto& Child : ChildrenCopy)
    {
        Child->Release();
        delete Child;
    }
}

void SWindow::LoadConfig(const TMap<FString, FString>& config)
{
    for (const auto& Child : Children)
    {
        Child->LoadConfig(config);
    }
}

void SWindow::SaveConfig(TMap<FString, FString>& config) const
{
    for (const auto& Child : Children)
    {
        Child->SaveConfig(config);
    }
}

bool SWindow::IsHover(FVector2D coord) const
{
    return Rect.Contains(coord);
}

bool SWindow::IsHover(FPoint coord) const
{
    return Rect.Contains(coord);
}

void SWindow::OnPressed(FPoint coord)
{
    for (const auto& Child : Children)
    {
        Child->OnPressed(coord);
    }
}

void SWindow::OnReleased() 
{
    for (const auto& Child : Children)
    {
        Child->OnReleased();
    }
}
