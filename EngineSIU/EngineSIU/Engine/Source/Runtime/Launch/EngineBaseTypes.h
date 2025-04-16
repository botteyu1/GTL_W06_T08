#pragma once
#include "HAL/PlatformType.h"


enum EViewModeIndex : uint8
{
    VMI_LitGouraud,
    VMI_LitLambert,
    VMI_LitBlinnPhong,
    VMI_Unlit,
    VMI_Wireframe,
    VMI_SceneDepth,
    VMI_WorldNormal,
    VMI_WorldPos,
    VMI_Attenuation,
};

namespace EEngineShowFlags
{
    enum Type : uint64
    {
        None = 0,
        SF_AABB = 1ULL << 0,
        SF_Primitives = 1ULL << 1,
        SF_BillboardText = 1ULL << 2,
        SF_UUIDText = 1ULL << 3,
        SF_Fog = 1ULL << 4,
        SF_LightHeatMap = 1ULL << 5,
        SF_Icons = 1ULL << 6,
        SF_LightVisualize = 1ULL << 7,
    };
}

enum ELevelViewportType : uint8
{
    LVT_Perspective = 0,

    /** Top */
    LVT_OrthoXY = 1,
    /** Bottom */
    LVT_OrthoNegativeXY,
    /** Left */
    LVT_OrthoYZ,
    /** Right */
    LVT_OrthoNegativeYZ,
    /** Front */
    LVT_OrthoXZ,
    /** Back */
    LVT_OrthoNegativeXZ,

    LVT_MAX,
    LVT_None = 255,
};
