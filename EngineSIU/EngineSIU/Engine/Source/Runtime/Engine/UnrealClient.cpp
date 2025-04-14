#include "UnrealClient.h"
//#include "UnrealEd/EditorViewportClient.h"

void FViewport::Initialize()
{
}

void FViewport::InitializeViewport(const DXGI_SWAP_CHAIN_DESC& swapchaindesc)
{
    float width = (float)swapchaindesc.BufferDesc.Width;
    float height = (float)swapchaindesc.BufferDesc.Height;
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    switch (viewLocation)
    {
    case EViewScreenLocation::TopLeft:
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = halfWidth;
        viewport.Height = halfHeight;
        break;
    case EViewScreenLocation::TopRight:
        viewport.TopLeftX = halfWidth;
        viewport.TopLeftY = 0.0f;
        viewport.Width = halfWidth;
        viewport.Height = halfHeight;
        break;
    case EViewScreenLocation::BottomLeft:
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = halfHeight;
        viewport.Width = halfWidth;
        viewport.Height = halfHeight;
        break;
    case EViewScreenLocation::BottomRight:
        viewport.TopLeftX = halfWidth;
        viewport.TopLeftY = halfHeight;
        viewport.Width = halfWidth;
        viewport.Height = halfHeight;
        break;
    default:
        break;
    }
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
}

void FViewport::ResizeViewport(FSlateRect Top, FSlateRect Bottom, FSlateRect Left, FSlateRect Right)
{
    switch (viewLocation)
    {
    case EViewScreenLocation::TopLeft:
        viewport.TopLeftX = Left.Left;
        viewport.TopLeftY = Top.Top;
        viewport.Width = Left.GetWidth();
        viewport.Height = Top.GetHeight();
        break;
    case EViewScreenLocation::TopRight:
        viewport.TopLeftX = Right.Left;
        viewport.TopLeftY = Top.Top;
        viewport.Width = Right.GetWidth();
        viewport.Height = Top.GetHeight();
        break;
    case EViewScreenLocation::BottomLeft:
        viewport.TopLeftX = Left.Left;
        viewport.TopLeftY = Bottom.Top;
        viewport.Width = Left.GetWidth();
        viewport.Height = Bottom.GetHeight();
        break;
    case EViewScreenLocation::BottomRight:
        viewport.TopLeftX = Right.Left;
        viewport.TopLeftY = Bottom.Top;
        viewport.Width = Right.GetWidth();
        viewport.Height = Bottom.GetHeight();
        break;
    default:
        break;
    }
}

void FViewport::ResizeViewport(FSlateRect newRect)
{
    viewport.TopLeftX = newRect.Left;
    viewport.TopLeftY = newRect.Top;
    viewport.Width = newRect.GetWidth();
    viewport.Height = newRect.GetHeight();
}

