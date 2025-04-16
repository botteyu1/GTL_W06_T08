#pragma once
#include "Launch/EngineLoop.h"

namespace RendererHelpers {
    
    inline FMatrix CalculateMVP(const FMatrix& Model, const FMatrix& View, const FMatrix& Projection) {
        return Model * View * Projection;
    }

    inline FMatrix CalculateNormalMatrix(const FMatrix& Model) {
        return FMatrix::Transpose(FMatrix::Inverse(Model));
    }
}

namespace MaterialUtils
{
    inline void UpdateMaterial(FDXDBufferManager* BufferManager, FGraphicsDevice* Graphics, const FObjMaterialInfo& MaterialInfo) {
        FMaterialConstants data;
        data.DiffuseColor = MaterialInfo.Diffuse;
        data.AmbientColor = MaterialInfo.Ambient;
        data.SpecularColor = MaterialInfo.Specular;
        data.EmmisiveColor = MaterialInfo.Emissive;
        data.Sharpness = MaterialInfo.Sharpness;
        data.SpecularScalar = MaterialInfo.SpecularScalar;
        data.DensityScalar = MaterialInfo.DensityScalar;
        data.Illum = MaterialInfo.IlluminanceModel;
        data.TransparencyScalar = MaterialInfo.TransparencyScalar;
        data.TextureFlag = MaterialInfo.TextureFlag;

        BufferManager->UpdateConstantBuffer(TEXT("FMaterialConstants"), data);


        for (auto& [TextureFlag, TextureData] : MaterialInfo.TextureData)
        {
            uint32 flag = TextureFlag;
            int index = 0;
            while ((flag & 1) == 0) {
                flag >>= 1;
                ++index;
            }
            
            std::shared_ptr<FTexture> texture = FEngineLoop::ResourceManager.GetTexture(TextureData.Path);
            Graphics->DeviceContext->PSSetShaderResources(index, 1, &texture->TextureSRV);
            Graphics->DeviceContext->VSSetShaderResources(index, 1, &texture->TextureSRV);
            
            Graphics->DeviceContext->PSSetSamplers(index, 1, &texture->SamplerState);
            Graphics->DeviceContext->VSSetSamplers(index, 1, &texture->SamplerState);
        }

        // TODO: 해제하는 것이 안전하나 TextureFlag가 있어서 쉐이더에서 체크 가능함.
        // {
        //     ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        //     ID3D11SamplerState* nullSampler[1] = { nullptr };
        //     Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);
        //     Graphics->DeviceContext->PSSetSamplers(0, 1, nullSampler);
        // }
    }
}
