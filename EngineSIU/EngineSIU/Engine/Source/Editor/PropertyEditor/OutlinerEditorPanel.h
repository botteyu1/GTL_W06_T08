#pragma once
#include "UnrealEd/EditorPanel.h"

class OutlinerEditorPanel : public UEditorPanel
{
public:
    OutlinerEditorPanel() = default;

public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;
};
