#pragma once
#include <sstream>

#include "SlateCore/Widgets/SWindow.h"

class SSplitter : public SWindow
{
public:
    SWindow* SideLT; // Left or Top
    SWindow* SideRB; // Right or Bottom
    
    virtual void Initialize(FSlateRect initRect) override;

    virtual void OnDragStart(const FPoint& mousePos) { /* 초기화 */ }
    virtual void OnDrag(const FPoint& delta) = 0; // 가로/세로에 따라 구현 다름.
    virtual void OnResize(float width, float height) override;
    virtual bool OnPressed(FPoint coord) override;
    virtual bool OnReleased() override;

    virtual void OnDragEnd() {

    }

    virtual void LoadConfig(const TMap<FString, FString>& config);
    virtual void SaveConfig(TMap<FString, FString>& config) const;

    // 스플리터가 포함된 영역에 따라 자식 창의 Rect를 재계산하는 함수
    virtual void UpdateChildRects() = 0;

    template <typename T>
    T GetValueFromConfig(const TMap<FString, FString>& config, const FString& key, T defaultValue) {
        if (const FString* Value = config.Find(key))
        {
            std::istringstream iss(**Value);
            T value;
            if (iss >> value)
            {
                return value;
            }
        }
        return defaultValue;
    }
};

class SSplitterH : public SSplitter
{
public:
    virtual void Initialize(FSlateRect initRect) override;
    virtual void OnResize(float width, float height) override;

    virtual void LoadConfig(const TMap<FString, FString>& config) override;
    virtual void SaveConfig(TMap<FString, FString>& config) const override;

    virtual void OnDrag(const FPoint& delta) override {
        // 수평 스플리터의 경우, 좌우로 이동
        Rect.Left += delta.X;

        UpdateChildRects();
    }

    virtual void UpdateChildRects() override
    {
        if (SideLT)
        {
            SideLT->Rect.SetWidth(Rect.Left - SideLT->Rect.Left);
        }
        if (SideRB)
        {
            float prevleftTopX = SideRB->Rect.Left;
            SideRB->Rect.Left = Rect.Left + Rect.GetWidth();
            SideRB->Rect.SetWidth(SideRB->Rect.GetWidth() + prevleftTopX - SideRB->Rect.Left);
        }
    }
};

class SSplitterV : public SSplitter
{
public:
    virtual void Initialize(FSlateRect initRect) override;
    virtual void OnResize(float width, float height)    override;

    virtual void LoadConfig(const TMap<FString, FString>& config)   override;
    virtual void SaveConfig(TMap<FString, FString>& config) const   override;

    virtual void OnDrag(const FPoint& delta) override {

        Rect.Top += delta.Y;
        UpdateChildRects();
    }

    virtual void UpdateChildRects() override {

        if (SideLT)
        {
            SideLT->Rect.SetHeight(Rect.Top - SideLT->Rect.Top);
        }
        if (SideRB)
        {
            float prevleftTopY = SideRB->Rect.Top;
            SideRB->Rect.Top = Rect.Top + Rect.GetHeight();
            SideRB->Rect.SetHeight(SideRB->Rect.GetHeight() + prevleftTopY - SideRB->Rect.Top);
        }
    }
};
