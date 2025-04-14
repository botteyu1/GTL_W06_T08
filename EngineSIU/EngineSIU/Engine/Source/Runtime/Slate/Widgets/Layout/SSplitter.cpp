#include "SSplitter.h"
#include "EngineLoop.h"

extern FEngineLoop GEngineLoop;

void SSplitter::Initialize(FSlateRect initRect)
{
    SWindow::Initialize(initRect);
    if (SideLT == nullptr)
    {
        SideLT = new SWindow();
    }
    if (SideRB == nullptr)
    {
        SideRB = new SWindow();
    }
}

void SSplitter::OnResize(float width, float height)
{
}

bool SSplitter::OnPressed(FPoint coord)
{
    if (!IsHover(coord))
        return false;
    
    return bIsPressed = true;
}

bool SSplitter::OnReleased()
{
    bIsPressed = false;
    return false;
}

void SSplitter::LoadConfig(const TMap<FString, FString>& config)
{
}

void SSplitter::SaveConfig(TMap<FString, FString>& config) const
{
}

void SSplitterH::Initialize(FSlateRect initRect)
{
    SWindow::Initialize(initRect);
    if (SideLT)
    {
        SideLT->Initialize(FSlateRect(0.0f, 0.0f, initRect.Left, initRect.GetHeight()));
    }
    if (SideRB)
    {
        SideRB->Initialize(FSlateRect(initRect.Left + initRect.GetHeight(), 0.0f, initRect.Left, initRect.GetHeight()));
    }
}

void SSplitterH::OnResize(float width, float height)
{
    Rect.SetHeight(height);
    Rect.Left *= width;
    if (SideLT)
    {
        SideLT->Rect.SetHeight(height);
    }
    if (SideRB)
    {
        SideRB->Rect.Left *= width;
        SideRB->Rect.SetWidth(SideRB->Rect.GetWidth() * width);
        SideLT->Rect.SetHeight(height);

    }
    UpdateChildRects();
}

void SSplitterH::LoadConfig(const TMap<FString, FString>& config)
{
    // 각 키에 대해 기본값을 지정 (예: 기본 위치 및 크기)
    Rect.Left = GetValueFromConfig(config, "SplitterH.X", FEngineLoop::GraphicDevice.screenWidth *0.5f);
    Rect.Top = GetValueFromConfig(config, "SplitterH.Y", 0.0f);

    Rect.SetWidth(GetValueFromConfig(config, "SplitterH.Width", 20.0f));
    Rect.SetHeight(GetValueFromConfig(config, "SplitterH.Height", 10.0f)); // 수평 스플리터는 높이 고정
    
    Rect.Left *= FEngineLoop::GraphicDevice.screenWidth /GetValueFromConfig(config, "SplitterV.Width", 1000.0f);
}

void SSplitterH::SaveConfig(TMap<FString, FString>& config) const
{
    config["SplitterH.X"] = std::to_string(Rect.Left);
    config["SplitterH.Y"] = std::to_string(Rect.Top);
    config["SplitterH.Width"] = std::to_string(Rect.GetWidth());
    config["SplitterH.Height"] = std::to_string(Rect.GetHeight());
}

void SSplitterV::Initialize(FSlateRect initRect)
{
    __super::Initialize(initRect);
    if (SideLT)
    {
        SideLT->Initialize(FSlateRect(0.0f, 0.0f, initRect.GetWidth(), initRect.Top));
    }
    if (SideRB)
    {
        SideRB->Initialize(FSlateRect(0.0f, initRect.Top + initRect.GetHeight(), initRect.GetWidth(), initRect.Top));
    }
}

void SSplitterV::OnResize(float width, float height)
{
    Rect.SetWidth(width);
    Rect.Top *= height;
    if (SideLT)
    {
        SideLT->Rect.SetWidth(width);
    }
    if (SideRB)
    {
        SideRB->Rect.Top *= height;
        SideRB->Rect.SetHeight(SideRB->Rect.GetHeight() * height);
        SideRB->Rect.SetWidth(width);
    }
    UpdateChildRects();
}

void SSplitterV::LoadConfig(const TMap<FString, FString>& config)
{
    Rect.Left = GetValueFromConfig(config, "SplitterV.X", 0.0f);
    Rect.Top = GetValueFromConfig(config, "SplitterV.Y", FEngineLoop::GraphicDevice.screenHeight * 0.5f);
    Rect.SetWidth(GetValueFromConfig(config, "SplitterV.Width", 10)); // 수직 스플리터는 너비 고정
    Rect.SetHeight(GetValueFromConfig(config, "SplitterV.Height", 20));

    Rect.Top *= FEngineLoop::GraphicDevice.screenHeight / GetValueFromConfig(config, "SplitterH.Height", 1000.0f);

}

void SSplitterV::SaveConfig(TMap<FString, FString>& config) const
{
    config["SplitterV.X"] = std::to_string(Rect.Left);
    config["SplitterV.Y"] = std::to_string(Rect.Top);
    config["SplitterV.Width"] = std::to_string(Rect.GetWidth());
    config["SplitterV.Height"] = std::to_string(Rect.GetHeight());
}
