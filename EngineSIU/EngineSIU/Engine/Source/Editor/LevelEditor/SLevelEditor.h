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
    void Tick(float DeltaTime);
    void Release();

    void SelectViewport(FVector2D InPoint);

    void ResizeLevelEditor();
    void ResizeViewports();
    void SetEnableMultiViewport(bool bIsEnable);
    bool IsMultiViewport() const;

private:
    SSplitterH* HSplitter;
    SSplitterV* VSplitter;
    UWorld* World;
    TArray<std::shared_ptr<FEditorViewportClient>> ViewportClients;

    int32 FocusedIndex;
    
    bool bLButtonDown = false;
    bool bRButtonDown = false;
    
    /** 우클릭 시 캡처된 마우스 커서의 초기 위치 (스크린 좌표계) */
    FVector2D MousePinPosition;

    /** 우클릭이 눌려있는지 여부 */
    bool bIsPressedMouseRightButton = false;

    bool bMultiViewportMode;

    float EditorWidth;
    float EditorHeight;

public:
    inline TArray<std::shared_ptr<FEditorViewportClient>> GetViewportClients() { return ViewportClients; }
    inline std::shared_ptr<FEditorViewportClient> GetFocusedViewportClient() const { return ViewportClients[FocusedIndex]; }

    inline void SetViewportClient(const std::shared_ptr<FEditorViewportClient>& viewportClient)
    {
        int32 FindIndex = ViewportClients.Find(viewportClient);
        if (FindIndex != -1)
        {
            FocusedIndex = FindIndex;
        }
    }
    inline void SetViewportClient(int index) { FocusedIndex = index; }

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
