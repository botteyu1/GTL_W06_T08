#include "SLevelEditor.h"

#include <fstream>

#include "UnrealClient.h"
#include "WindowsCursor.h"
#include "Slate/Widgets/Layout/SSplitter.h"
#include "UnrealEd/EditorViewportClient.h"

extern FEngineLoop GEngineLoop;

#define VIEWPORTNUM 4


SLevelEditor::SLevelEditor() : bMultiViewportMode(false)
{
}

void SLevelEditor::Initialize()
{    
    FocusedIndex = 0;

    ViewportWindow = new SWindow();
    ViewportWindow->Rect = FSlateRect(
        FEngineLoop::GraphicDevice.ScreenWidth * AnchorMin.X + Padding.Left,
        FEngineLoop::GraphicDevice.ScreenHeight * AnchorMin.Y + Padding.Top,
        (FEngineLoop::GraphicDevice.ScreenWidth * AnchorMax.X) - Padding.Right,
        (FEngineLoop::GraphicDevice.ScreenHeight * AnchorMax.Y) - Padding.Bottom);

    VSplitter = new SSplitterV(10);
    HSplitter = new SSplitterH(10);
    
    VSplitter->Initialize(ViewportWindow);
    HSplitter->Initialize(ViewportWindow);
    
    ViewportWindow->AddChildren(VSplitter);
    ViewportWindow->AddChildren(HSplitter);


    // TODO: 각 Viewport와 Rect 매핑?
    for (size_t i = 0; i < VIEWPORTNUM; i++)
    {
        std::shared_ptr<FEditorViewportClient> EditorViewportClient = std::make_shared<FEditorViewportClient>();
        EditorViewportClient->Initialize(i);
        ViewportClients.Add(EditorViewportClient);
    }

    
    LoadConfig();
    
    SetEnableMultiViewport(bMultiViewportMode);
    ResizeLevelEditor();


    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    Handler->OnMouseDownDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        switch (InMouseEvent.GetEffectingButton())  // NOLINT(clang-diagnostic-switch-enum)
        {
        case EKeys::RightMouseButton:
        {
            if (!bIsPressedMouseRightButton && !InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
            {
                FWindowsCursor::SetShowMouseCursor(false);
                MousePinPosition = InMouseEvent.GetScreenSpacePosition();
                bIsPressedMouseRightButton = true;
            }
            break;
        }
        default:
            break;
        }

        // 마우스 이벤트가 일어난 위치의 뷰포트를 선택
        if (bMultiViewportMode)
        {
            POINT Point;
            GetCursorPos(&Point);
            ScreenToClient(GEngineLoop.AppWnd, &Point);
            FVector2D ClientPos = FVector2D{static_cast<float>(Point.x), static_cast<float>(Point.y)};
            SelectViewport(ClientPos);
            ViewportWindow->OnPressed({ClientPos.X, ClientPos.Y});
        }
    });

    Handler->OnMouseMoveDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        // Splitter 움직임 로직
        if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
        {            
            const auto& [DeltaX, DeltaY] = InMouseEvent.GetCursorDelta();
                        
            if (VSplitter->IsPressing())
            {
                VSplitter->OnDrag(FPoint(DeltaX, DeltaY));
            }
            if (HSplitter->IsPressing())
            {
                HSplitter->OnDrag(FPoint(DeltaX, DeltaY));
            }

            if (VSplitter->IsPressing() || HSplitter->IsPressing())
            {
                FEngineLoop::GraphicDevice.OnResize(GEngineLoop.AppWnd);
                ResizeViewports();
            }
        }
        
        // 멀티 뷰포트일 때, 커서 변경 로직
        if (
            bMultiViewportMode
            && !InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)
            && !InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)
        ) {
            // TODO: 나중에 커서가 Viewport 위에 있을때만 ECursorType::Crosshair로 바꾸게끔 하기
            // ECursorType CursorType = ECursorType::Crosshair;
            ECursorType CursorType = ECursorType::Arrow;
            POINT Point;

            GetCursorPos(&Point);
            ScreenToClient(GEngineLoop.AppWnd, &Point);
            FVector2D ClientPos = FVector2D{static_cast<float>(Point.x), static_cast<float>(Point.y)};
            const bool bIsVerticalHovered = VSplitter->IsHover(ClientPos);
            const bool bIsHorizontalHovered = HSplitter->IsHover(ClientPos);

            if (bIsHorizontalHovered && bIsVerticalHovered)
            {
                CursorType = ECursorType::ResizeAll;
            }
            else if (bIsHorizontalHovered)
            {
                CursorType = ECursorType::ResizeLeftRight;
            }
            else if (bIsVerticalHovered)
            {
                CursorType = ECursorType::ResizeUpDown;
            }
            FWindowsCursor::SetMouseCursor(CursorType);
        }
    });

    Handler->OnMouseUpDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        switch (InMouseEvent.GetEffectingButton())  // NOLINT(clang-diagnostic-switch-enum)
        {
        case EKeys::RightMouseButton:
        {
            if (bIsPressedMouseRightButton)
            {
                FWindowsCursor::SetShowMouseCursor(true);
                FWindowsCursor::SetPosition(
                    static_cast<int32>(MousePinPosition.X),
                    static_cast<int32>(MousePinPosition.Y)
                );
                bIsPressedMouseRightButton = false;
            }
            return;
        }

        // Viewport 선택 로직
        case EKeys::LeftMouseButton:
        {
            ViewportWindow->OnReleased();
            return;
        }

        default:
            return;
        }
    });

    Handler->OnRawMouseInputDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        auto FocusedViewportClient = GetFocusedViewportClient();
        
        // Mouse Move 이벤트 일때만 실행
        if (
            InMouseEvent.GetInputEvent() == IE_Axis
            && InMouseEvent.GetEffectingButton() == EKeys::Invalid
        )
        {
            // 에디터 카메라 이동 로직
            if (bIsPressedMouseRightButton)
            {
                FocusedViewportClient->MouseMove(InMouseEvent);
            }
        }

        // 마우스 휠 이벤트
        else if (InMouseEvent.GetEffectingButton() == EKeys::MouseWheelAxis)
        {
            // 카메라 속도 조절
            if (bIsPressedMouseRightButton && FocusedViewportClient->IsPerspective())
            {
                const float CurrentSpeed = FocusedViewportClient->GetCameraSpeedScalar();
                const float Adjustment = FMath::Sign(InMouseEvent.GetWheelDelta()) * FMath::Loge(CurrentSpeed + 1.0f) * 0.5f;

                FocusedViewportClient->SetCameraSpeedScalar(CurrentSpeed + Adjustment);
            }
        }
    });

    Handler->OnMouseWheelDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        auto FocusedViewportClient = GetFocusedViewportClient();
        
        // 뷰포트에서 앞뒤 방향으로 화면 이동
        if (FocusedViewportClient->IsPerspective())
        {
            if (!bIsPressedMouseRightButton)
            {
                const FVector CameraLoc = FocusedViewportClient->ViewTransformPerspective.GetLocation();
                const FVector CameraForward = FocusedViewportClient->ViewTransformPerspective.GetForwardVector();
                FocusedViewportClient->ViewTransformPerspective.SetLocation(
                    CameraLoc + CameraForward * InMouseEvent.GetWheelDelta() * 50.0f
                );
            }
        }
        else
        {
            FEditorViewportClient::SetOthoSize(-InMouseEvent.GetWheelDelta());
        }
    });

    Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
    {
        GetFocusedViewportClient()->InputKey(InKeyEvent);
    });

    Handler->OnKeyUpDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
    {
        GetFocusedViewportClient()->InputKey(InKeyEvent);
    });
}

void SLevelEditor::Tick(float DeltaTime)
{
    for (std::shared_ptr<FEditorViewportClient> Viewport : ViewportClients)
    {
        Viewport->Tick(DeltaTime);
    }
}

void SLevelEditor::Release()
{
    SaveConfig();

    ViewportWindow->Release();
    delete ViewportWindow;
}

void SLevelEditor::SelectViewport(FVector2D InPoint)
{
    for (const auto& ViewportClient : ViewportClients)
    {
        if (ViewportClient->IsSelected(InPoint))
        {
            SetViewportClient(ViewportClient);
            break;
        }
    }
}

void SLevelEditor::ResizeLevelEditor()
{
    double PrevWidth = ViewportWindow->Rect.GetWidth();
    double PrevHeight = ViewportWindow->Rect.GetHeight();
    //ViewportWindow->Rect.SetWidth(FEngineLoop::GraphicDevice.ScreenWidth);
    //ViewportWindow->Rect.SetHeight(FEngineLoop::GraphicDevice.ScreenHeight);



    FSlateRect TargetRect = FSlateRect(
    FEngineLoop::GraphicDevice.ScreenWidth * AnchorMin.X + Padding.Left,
FEngineLoop::GraphicDevice.ScreenHeight * AnchorMin.Y + Padding.Top,
(FEngineLoop::GraphicDevice.ScreenWidth * AnchorMax.X) - Padding.Right,
(FEngineLoop::GraphicDevice.ScreenHeight * AnchorMax.Y) - Padding.Bottom);
    
    FVector2D RectSize = FVector2D(TargetRect.GetWidth(), TargetRect.GetHeight());
    
    // foreach Children -> Splitter->OnResize()
    ViewportWindow->OnResize(RectSize.X / PrevWidth, RectSize.Y / PrevHeight);
    ResizeViewports();
}

void SLevelEditor::ResizeViewports()
{
    if (bMultiViewportMode)
    {
        // 각 바인딩 된 Viewport로
        for (const auto& ViewportClient : ViewportClients)
        {
            ViewportClient->ResizeViewport(VSplitter->SideLT->Rect, VSplitter->SideRB->Rect,
                HSplitter->SideLT->Rect, HSplitter->SideRB->Rect);
        }
    }
    else
    {
        GetFocusedViewportClient()->ResizeViewport(ViewportWindow->Rect);
    }
}

void SLevelEditor::SetEnableMultiViewport(bool bIsEnable)
{
    bMultiViewportMode = bIsEnable;
    ResizeViewports();
}

bool SLevelEditor::IsMultiViewport() const
{
    return bMultiViewportMode;
}

void SLevelEditor::LoadConfig()
{
    auto config = ReadIniFile(IniFilePath);
    FEditorViewportClient::Pivot.X = GetValueFromConfig(config, "OrthoPivotX", 0.0f);
    FEditorViewportClient::Pivot.Y = GetValueFromConfig(config, "OrthoPivotY", 0.0f);
    FEditorViewportClient::Pivot.Z = GetValueFromConfig(config, "OrthoPivotZ", 0.0f);
    FEditorViewportClient::OrthoSize = GetValueFromConfig(config, "OrthoZoomSize", 10.0f);
    
    // FEngineLoop::GraphicDevice.ScreenPosX = GetValueFromConfig(config, "ScreenPosX", FEngineLoop::GraphicDevice.ScreenPosX);
    // FEngineLoop::GraphicDevice.ScreenPosY = GetValueFromConfig(config, "ScreenPosY", FEngineLoop::GraphicDevice.ScreenPosY);
    //
    // FEngineLoop::GraphicDevice.ScreenWidth = GetValueFromConfig(config, "ScreenWidth", FEngineLoop::GraphicDevice.ScreenWidth);
    // FEngineLoop::GraphicDevice.ScreenHeight = GetValueFromConfig(config, "ScreenHeight", FEngineLoop::GraphicDevice.ScreenHeight);
    
    SetViewportClient(GetValueFromConfig(config, "ActiveViewportIndex", 0));
    bMultiViewportMode = GetValueFromConfig(config, "bMutiView", false);
    for (const auto& ViewportClient : ViewportClients)
    {
        ViewportClient->LoadConfig(config);
    }

    ViewportWindow->LoadConfig(config);
    
}

void SLevelEditor::SaveConfig()
{
    TMap<FString, FString> config;
    ViewportWindow->SaveConfig(config);
    for (const auto& ViewportClient : ViewportClients)
    {
        ViewportClient->SaveConfig(config);
    }
    GetFocusedViewportClient()->SaveConfig(config);
    config["bMutiView"] = std::to_string(bMultiViewportMode);
    config["ActiveViewportIndex"] = std::to_string(FocusedIndex);
    // config["ScreenPosX"] = std::to_string(FEngineLoop::GraphicDevice.ScreenPosX);
    // config["ScreenPosY"] = std::to_string(FEngineLoop::GraphicDevice.ScreenPosY);
    // config["ScreenWidth"] = std::to_string(FEngineLoop::GraphicDevice.ScreenWidth);
    // config["ScreenHeight"] = std::to_string(FEngineLoop::GraphicDevice.ScreenHeight);
    config["OrthoPivotX"] = std::to_string(FEditorViewportClient::Pivot.X);
    config["OrthoPivotY"] = std::to_string(FEditorViewportClient::Pivot.Y);
    config["OrthoPivotZ"] = std::to_string(FEditorViewportClient::Pivot.Z);
    config["OrthoZoomSize"] = std::to_string(FEditorViewportClient::OrthoSize);
    WriteIniFile(IniFilePath, config);
}

TMap<FString, FString> SLevelEditor::ReadIniFile(const FString& filePath)
{
    TMap<FString, FString> config;
    std::ifstream file(*filePath);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '[' || line[0] == ';') continue;
        std::istringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            config[key] = value;
        }
    }
    return config;
}

void SLevelEditor::WriteIniFile(const FString& filePath, const TMap<FString, FString>& config)
{
    std::ofstream file(*filePath);
    for (const auto& pair : config) {
        file << *pair.Key << "=" << *pair.Value << "\n";
    }
}

