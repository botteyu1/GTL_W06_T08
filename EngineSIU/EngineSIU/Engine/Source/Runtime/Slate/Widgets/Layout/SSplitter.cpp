#include "SSplitter.h"
#include "EngineLoop.h"

extern FEngineLoop GEngineLoop;

SSplitter::SSplitter(float SplitterHalfSize) : SplitterHalfSize(SplitterHalfSize)
{
}

SSplitter::~SSplitter()
{
}

void SSplitter::OnPressed(FPoint coord)
{
    if (!IsHover(coord))
    {
        bIsPressed = false;
    }
    
    bIsPressed = true;
}

void SSplitter::OnReleased()
{
    bIsPressed = false;
}

void SSplitter::LoadConfig(const TMap<FString, FString>& config)
{
    __super::LoadConfig(config);
}

void SSplitter::SaveConfig(TMap<FString, FString>& config) const
{
    __super::SaveConfig(config);
}

void SSplitterH::Initialize(SWindow* InParent)
{
    __super::Initialize(InParent);

    FVector2D Center = InParent->Rect.GetCenter();

    Rect = FSlateRect(Center.X - SplitterHalfSize, InParent->Rect.Top, Center.X + SplitterHalfSize, InParent->Rect.Bottom);
    
    SideLT = new SWindow();
    SideRB = new SWindow();

    AddChildren(SideRB);
    AddChildren(SideLT);
    
    UpdateChildRects();
}

// Vertical 자르기 (Horizontal로 늘어남)
void SSplitterH::OnResize(double DeltaWidthRatio, double DeltaHeightRatio)
{
    Rect.Left *= DeltaWidthRatio;
    Rect.Right *= DeltaWidthRatio;

    Rect.Top *= DeltaHeightRatio;
    Rect.Bottom *= DeltaHeightRatio;
    
    UpdateChildRects();
}

void SSplitterH::UpdateChildRects()
{
    SideLT->Rect = (FSlateRect(Parent->Rect.Left, Rect.Top, Rect.Left, Rect.Bottom));
    SideRB->Rect = (FSlateRect(Rect.Right, Rect.Top, Parent->Rect.Right,  Rect.Bottom));
}

void SSplitterH::LoadConfig(const TMap<FString, FString>& config)
{
    __super::LoadConfig(config);
    
    // 각 키에 대해 기본값을 지정 (예: 기본 위치 및 크기)
    Rect.Left = GetValueFromConfig(config, "SplitterH.Left", Parent->Rect.GetCenter().X);
    Rect.Top = GetValueFromConfig(config, "SplitterH.Top", Parent->Rect.Top);
    Rect.Bottom = GetValueFromConfig(config, "SplitterH.Bottom", Parent->Rect.Bottom); // 수직 스플리터는 너비 고정
    Rect.Right = GetValueFromConfig(config, "SplitterH.Right", Rect.Left + 20);
    
    //Rect.Left *= FEngineLoop::GraphicDevice.ScreenWidth /GetValueFromConfig(config, "SplitterV.Width", 1000.0f);
}

void SSplitterH::SaveConfig(TMap<FString, FString>& config) const
{
    __super::SaveConfig(config);
    
    config["SplitterH.Left"] = std::to_string(Rect.Left);
    config["SplitterH.Top"] = std::to_string(Rect.Top);
    config["SplitterH.Right"] = std::to_string(Rect.Right);
    config["SplitterH.Bottom"] = std::to_string(Rect.Bottom);
}

void SSplitterV::Initialize(SWindow* InParent)
{
    __super::Initialize(InParent);

    FVector2D Center = InParent->Rect.GetCenter();
    
    Rect = FSlateRect(InParent->Rect.Left, Center.Y - SplitterHalfSize, InParent->Rect.Right, Center.Y + SplitterHalfSize);

    SideLT = new SWindow();
    SideRB = new SWindow();

    AddChildren(SideRB);
    AddChildren(SideLT);

    UpdateChildRects();
}

// Horizontal 자르기 (Vertical로 늘어남)
void SSplitterV::OnResize(double DeltaWidthRatio, double DeltaHeightRatio)
{
    Rect.Left *= DeltaWidthRatio;
    Rect.Right *= DeltaWidthRatio;

    Rect.Top *= DeltaHeightRatio;
    Rect.Bottom *= DeltaHeightRatio;
    
    UpdateChildRects();
}

void SSplitterV::UpdateChildRects()
{
    SideLT->Rect = (FSlateRect(Rect.Left, Parent->Rect.Top, Rect.Right, Rect.Top));
    SideRB->Rect = (FSlateRect(Rect.Left, Rect.Bottom, Rect.Right,  Parent->Rect.Bottom));
}

void SSplitterV::LoadConfig(const TMap<FString, FString>& config)
{
    __super::LoadConfig(config);

    Rect.Left = GetValueFromConfig(config, "SplitterV.Left", Parent->Rect.Left);
    Rect.Top = GetValueFromConfig(config, "SplitterV.Top", Parent->Rect.GetCenter().Y);
    Rect.Bottom = GetValueFromConfig(config, "SplitterV.Bottom", Rect.Top + 20); // 수직 스플리터는 너비 고정
    Rect.Right = GetValueFromConfig(config, "SplitterV.Right", Parent->Rect.Right);

    //Rect.Top *= FEngineLoop::GraphicDevice.ScreenHeight / GetValueFromConfig(config, "SplitterH.Height", 1000.0f);
}

void SSplitterV::SaveConfig(TMap<FString, FString>& config) const
{
    __super::SaveConfig(config);
    
    config["SplitterV.Left"] = std::to_string(Rect.Left);
    config["SplitterV.Top"] = std::to_string(Rect.Top);
    config["SplitterV.Right"] = std::to_string(Rect.Right);
    config["SplitterV.Bottom"] = std::to_string(Rect.Bottom);
}
