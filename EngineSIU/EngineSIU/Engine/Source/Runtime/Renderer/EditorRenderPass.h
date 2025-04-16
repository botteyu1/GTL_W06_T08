 #pragma once

#define _TCHAR_DEFINED
#include <d3d11.h>
#include "IRenderPass.h"
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"
#include "Container/Set.h"
#include "RenderResources.h"

class FGraphicsDevice;
class UWorld;
class FEditorViewportClient;
class FRenderer;
class FDXDBufferManager;
class FDXDShaderManager;


class FEditorRenderPass
{
public:
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager);
    void Render(std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void SetGridParameter(float Spacing, uint32 GridCount);
    void Release();
    static const float IconScale;

private:
    ID3D11DeviceContext* DeviceContext;
    FGraphicsDevice* Graphics;
    FDXDBufferManager* BufferManager;
    FDXDShaderManager* ShaderManager;

    FRenderResourcesDebug Resources;

    void CreateShaders();
    //void Preparehader(FShaderResource ShaderResource) const;
    void ReleaseShaders();

    void CreateBuffers();
    void CreateConstantBuffers();

    void LazyLoad();

    void PrepareRendertarget();
    void PrepareComponents();

    void PrepareConstantbufferGlobal();
    void UpdateConstantbufferGlobal(FConstantBufferCamera Buffer);

    //// Gizmo 관련 함수
    //void RenderGizmos(const UWorld* World);
    //void PrepareShaderGizmo();
    //void PrepareConstantbufferGizmo();

    // Axiss
    void RenderAxis();

    // AABB
    void RenderAABBInstanced(std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void PrepareConstantbufferAABB();
    void UdpateConstantbufferAABBInstanced(TArray<FConstantBufferDebugAABB> Buffer);

    // Sphere
    void RenderPointlightInstanced(std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void PrepareConstantbufferPointlight();
    void UdpateConstantbufferPointlightInstanced(TArray<FConstantBufferDebugSphere> Buffer);

    // Cone
    void RenderSpotlightInstanced(std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void PrepareConstantbufferSpotlight();
    void UdpateConstantbufferSpotlightInstanced(TArray<FConstantBufferDebugCone> Buffer);

    // Grid
    void RenderGrid(std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void PrepareConstantbufferGrid();
    void UdpateConstantbufferGrid(FConstantBufferDebugGrid Buffer);

    // Icon
    void RenderIcons(std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void PrepareConstantbufferIcon();
    void UdpateConstantbufferIcon(FConstantBufferDebugIcon Buffer);
    void UdpateConstantbufferIconInstanced(TArray<FConstantBufferDebugIcon> Buffers);
    void UpdateTextureIcon(IconType type);

    // Arrow
    void RenderArrows();
    void PrepareConstantbufferArrow();
    void UdpateConstantbufferArrow(FConstantBufferDebugArrow Buffer);

    void UpdateShaders();

    const uint32 ConstantBufferSizeAABB = 8;
    const uint32 ConstantBufferSizeSphere = 1024;
    const uint32 ConstantBufferSizeCone = 2;
    const uint32 ConstantBufferSizeIcon = 1024;
    const uint32 NumSegments = 16; // Shaderconstants.h의 defines[]에서도 수정필요
    FConstantBufferDebugGrid CurrentGridSettings;

    const std::wstring ShaderPath = L"Shaders/EditorShader.hlsl";
    const std::wstring ShaderNameAxis = L"EditorRenderAxis";
    const std::wstring ShaderNameAABB = L"EditorRenderAABB";
    const std::wstring ShaderNameGrid = L"EditorRenderGrid";
    const std::wstring ShaderNameIcon = L"EditorRenderIcon";
    const std::wstring ShaderNameSphere = L"EditorRenderSphere";
    const std::wstring ShaderNameCone = L"EditorRenderCone";
    const std::wstring ShaderNameArrow = L"EditorRenderArrow";
    const std::wstring VertexBufferNameBox = L"BoxFrameOnly";
    const std::wstring VertexBufferNameCone = L"ConeFrameOnly";

};

