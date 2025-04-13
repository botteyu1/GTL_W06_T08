#pragma once
#include "Container/Array.h"
#include "Container/Map.h"


class FEditorViewportClient;
class UWorld;
class SSplitterV;
class SSplitterH;

class SLevelEditor
{
public:
    SLevelEditor();

    void Initialize();
    void Tick(double deltaTime);
    void Input();
    void Release();
    
    void SelectViewport(POINT point);
    void OnResize();
    void ResizeViewports();
    void OnMultiViewport();
    void OffMultiViewport();
    bool IsMultiViewport() const;

private:
    bool bInitialize;
    SSplitterH* HSplitter;
    SSplitterV* VSplitter;
    UWorld* World;
    TArray<std::shared_ptr<FEditorViewportClient>> ViewportClients;
    std::shared_ptr<FEditorViewportClient> ActiveViewportClient;

    bool bLButtonDown = false;
    bool bRButtonDown = false;
    
    bool bMultiViewportMode;

    POINT lastMousePos;
    float EditorWidth;
    float EditorHeight;

public:
    TArray<std::shared_ptr<FEditorViewportClient>> GetViewportClients() { return ViewportClients; }
    std::shared_ptr<FEditorViewportClient> GetFocusedViewportClient() const
    {
        return ActiveViewportClient;
    }
    void SetViewportClient(const std::shared_ptr<FEditorViewportClient>& viewportClient)
    {
        ActiveViewportClient = viewportClient;
    }
    void SetViewportClient(int index)
    {
        ActiveViewportClient = ViewportClients[index];
    }

    //Save And Load
private:
    const FString IniFilePath = "editor.ini";

public:
    void LoadConfig();
    void SaveConfig();

private:
    TMap<FString, FString> ReadIniFile(const FString& filePath);
    void WriteIniFile(const FString& filePath, const TMap<FString, FString>& config);

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
