#pragma once
#define _TCHAR_DEFINED
#include <d3d11.h>
#include <d3dcompiler.h>
#include "Container/Map.h"

struct FVertexShaderData
{
	ID3DBlob* VertexShaderCSO;
	ID3D11VertexShader* VertexShader;
};

class FDXDShaderManager
{
public:
	FDXDShaderManager() = default;
	FDXDShaderManager(ID3D11Device* Device);

	void ReleaseAllShader();

private:
	ID3D11Device* DXDDevice;

public:
	HRESULT AddVertexShader(const std::wstring& Key, const std::wstring& FileName);
	HRESULT AddVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines = nullptr);
	HRESULT AddInputLayout(const std::wstring& Key, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize);
	
	HRESULT AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* Defines = nullptr);

	HRESULT AddPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines = nullptr);

	ID3D11InputLayout* GetInputLayoutByKey(const std::wstring& Key) const;
	ID3D11VertexShader* GetVertexShaderByKey(const std::wstring& Key) const;
	ID3D11PixelShader* GetPixelShaderByKey(const std::wstring& Key) const;

    void SetVertexShader(const std::wstring& KeyName, ID3D11DeviceContext* DeviceContext) const;
    void SetVertexShaderAndInputLayout(const std::wstring KeyName, ID3D11DeviceContext* DeviceContext) const;
    void SetPixelShader(const std::wstring& KeyName, ID3D11DeviceContext* DeviceContext) const;
    void SetInputLayout(const std::wstring& KeyName, ID3D11DeviceContext* DeviceContext) const;

private:
	TMap<std::wstring, ID3D11InputLayout*> InputLayouts;
	TMap<std::wstring, ID3D11VertexShader*> VertexShaders;
	TMap<std::wstring, ID3D11PixelShader*> PixelShaders;

};

