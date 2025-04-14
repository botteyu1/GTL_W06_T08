#pragma once
#define _TCHAR_DEFINED
#include <d3d11.h>

#include "HAL/PlatformType.h"
#include "SlateCore/Layout/SlateRect.h"


enum class EViewScreenLocation : uint8
{
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

class FViewport
{
public:
    FViewport() = default;
    FViewport(EViewScreenLocation _viewLocation) : viewLocation(_viewLocation) {}

    void Initialize();
    void InitializeViewport(const DXGI_SWAP_CHAIN_DESC& swapchaindesc);
    void ResizeViewport(FSlateRect Top, FSlateRect Bottom, FSlateRect Left, FSlateRect Right);
    void ResizeViewport(FSlateRect newRect);

private:
    D3D11_VIEWPORT viewport;            // 뷰포트 정보
    EViewScreenLocation viewLocation;   // 뷰포트 위치
public:
    D3D11_VIEWPORT& GetViewport() { return viewport; }
    void SetViewport(const D3D11_VIEWPORT& _viewport) { viewport = _viewport; }

    FSlateRect GetFSlateRect() const
    {
        return FSlateRect(viewport.TopLeftX, viewport.TopLeftY, viewport.TopLeftX + viewport.Width, viewport.TopLeftY + viewport.Height);
    }
};
