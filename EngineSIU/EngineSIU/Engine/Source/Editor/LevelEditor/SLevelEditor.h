#pragma once
#include "Container/Array.h"
#include "Container/Map.h"
#include "Math/Vector.h"
#include "SlateCore/Widgets/SWindow.h"


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
    //FSlateRect Padding = FSlateRect(0, 50, 10, 0);
    FSlateRect Padding = FSlateRect(0, 0, 0, 0);
    // TODO: 다른 좋은 이름
    FVector2D AnchorMax = FVector2D(1, 1);
    FVector2D AnchorMin = FVector2D(0, 0);

private:
    SWindow* ViewportWindow = nullptr;

    SSplitterH* HSplitter;
    SSplitterV* VSplitter;
    
    TArray<std::shared_ptr<FEditorViewportClient>> ViewportClients;
    TMap<SWindow*, std::shared_ptr<FEditorViewportClient>> ViewportClientMap;

    int32 FocusedIndex;
    
    /** 우클릭 시 캡처된 마우스 커서의 초기 위치 (스크린 좌표계) */
    FVector2D MousePinPosition;

    /** 우클릭이 눌려있는지 여부 */
    bool bIsPressedMouseRightButton = false;

    bool bMultiViewportMode;

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
