#pragma once
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"

class ViewportControlEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    void CreateModifyButton(ImVec2 ButtonSize, ImFont* IconFont);
    void CreateFlagButton() const;
    void CreateSRTButton(ImVec2 IconSize, ImFont* IconFont) const;

    uint64 ConvertSelectionToFlags(const bool selected[]) const;
};
