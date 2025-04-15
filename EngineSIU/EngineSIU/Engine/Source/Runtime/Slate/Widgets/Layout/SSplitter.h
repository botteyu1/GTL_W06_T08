#pragma once
#include <sstream>

#include "SlateCore/Widgets/SWindow.h"

class SSplitter : public SWindow
{
public:
    SSplitter(float SplitterHalfSize);
    ~SSplitter() override;

public:    
    virtual void OnPressed(FPoint coord) override;
    virtual void OnReleased() override;

    virtual void LoadConfig(const TMap<FString, FString>& config) override;
    virtual void SaveConfig(TMap<FString, FString>& config) const override;
    virtual void ClampMinimumRegion() = 0;
public:
    virtual void OnDragStart(const FPoint& mousePos) { /* 초기화 */ }
    virtual void OnDrag(const FPoint& delta) = 0; // 가로/세로에 따라 구현 다름.


    // 스플리터가 포함된 영역에 따라 자식 창의 Rect를 재계산하는 함수
    virtual void UpdateChildRects() = 0;
    
    // Parent, Child로 있음
    SWindow* SideLT; // Left or Top
    SWindow* SideRB; // Right or Bottom
protected:

    float SplitterHalfSize;
    const float MinimumSplitterOffset = 10;

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
    SSplitterH(float SplitterHalfSize) : SSplitter(SplitterHalfSize) {}

public:
    void Initialize(SWindow* InParent) override;
    
    virtual void OnResize(double DeltaWidthRatio, double DeltaHeightRatio) override;
    virtual void UpdateChildRects() override;

    void LoadConfig(const TMap<FString, FString>& config) override;
    void SaveConfig(TMap<FString, FString>& config) const override;

    void ClampMinimumRegion() override;

    virtual void OnDrag(const FPoint& delta) override
    {
        // 수평 스플리터의 경우, 좌우로 이동
        Rect.Left += delta.X;
        Rect.Right += delta.X;

        ClampMinimumRegion();
        

        UpdateChildRects();
    }
};

class SSplitterV : public SSplitter
{
public:
    SSplitterV(float SplitterHalfSize) : SSplitter(SplitterHalfSize) {}
public:

    void Initialize(SWindow* InParent) override;
    
    virtual void OnResize(double DeltaWidthRatio, double DeltaHeightRatio) override;
    virtual void UpdateChildRects() override;
    
    void LoadConfig(const TMap<FString, FString>& config)   override;
    void SaveConfig(TMap<FString, FString>& config) const   override;

    void ClampMinimumRegion() override;
    
    virtual void OnDrag(const FPoint& delta) override
    {
        Rect.Bottom += delta.Y;
        Rect.Top += delta.Y;

        ClampMinimumRegion();
        
        UpdateChildRects();
    }
};
