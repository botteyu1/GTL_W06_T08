#pragma once
#include "Core/HAL/PlatformType.h"

enum EEditorLayoutTheme : uint8
{
    Dark,
    Light
};

class UImGuiManager
{
public:
    void Initialize(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);

    /**
     * This method must call after config loaded.
     */
    void PostInitialize();
    
    void BeginFrame() const;
    void EndFrame() const;
    void PreferenceStyle();
    void PreferenceStyle(EEditorLayoutTheme InTheme);
    void Shutdown();
};

