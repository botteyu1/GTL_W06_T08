#pragma once
#include <filesystem>
#define _TCHAR_DEFINED
#include <d3d11.h>
#include <d3dcompiler.h>
#include <set>

#include "Container/Map.h"


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
	HRESULT AddComputeShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines = nullptr);
	HRESULT AddInputLayout(const std::wstring& Key, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize);

    void AddVertexShader(const std::wstring& Key, ID3D11VertexShader* VerteShader);
    void AddVertexShader(const std::wstring& Key, ID3D11PixelShader* PixelShader);
    void AddInputLayout(const std::wstring& Key, ID3D11InputLayout* InputLayout);
	
	HRESULT AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* Defines = nullptr);

	HRESULT AddPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines = nullptr);

	ID3D11InputLayout* GetInputLayoutByKey(const std::wstring& Key) const;
	ID3D11VertexShader* GetVertexShaderByKey(const std::wstring& Key) const;
	ID3D11PixelShader* GetPixelShaderByKey(const std::wstring& Key) const;
    ID3D11ComputeShader* GetComputeShaderByKey(const std::wstring& Key) const;



    void RemoveInputLayoutByKey(const std::wstring& Key);
    void RemoveVertexShaderByKey(const std::wstring& Key);
    void RemovePixelShaderByKey(const std::wstring& Key);

    void SetVertexShader(const std::wstring& KeyName, ID3D11DeviceContext* DeviceContext) const;
    void SetVertexShaderAndInputLayout(const std::wstring KeyName, ID3D11DeviceContext* DeviceContext) const;
    void SetPixelShader(const std::wstring& KeyName, ID3D11DeviceContext* DeviceContext) const;
    void SetInputLayout(const std::wstring& KeyName, ID3D11DeviceContext* DeviceContext) const;

    HRESULT ReloadVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines);
    HRESULT ReloadPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines);
    HRESULT ReloadComputeShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines);
    HRESULT ReloadShaders(const std::wstring& VertexKey, const std::wstring& VertexFileName, const std::string& VertexEntryPoint, 
        const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* VertexDefines,
        const std::wstring& PixelKey, const std::wstring& PixelFileName, const std::string& PixelEntryPoint, const D3D_SHADER_MACRO* PixelDefines);

    HRESULT ReloadModifiedShaders(const std::wstring& VertexKey, const std::wstring& VertexFileName, const std::string& VertexEntryPoint,
        const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* VertexDefines,
        const std::wstring& PixelKey, const std::wstring& PixelFileName, const std::string& PixelEntryPoint, const D3D_SHADER_MACRO* PixelDefines);
    HRESULT ReloadModifiedComputeShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines);


private:
	TMap<std::wstring, ID3D11InputLayout*> InputLayouts;
	TMap<std::wstring, ID3D11VertexShader*> VertexShaders;
	TMap<std::wstring, ID3D11ComputeShader*> ComputeShaders;
	TMap<std::wstring, ID3D11PixelShader*> PixelShaders;
    
    TMap<std::wstring, TMap<std::wstring, std::filesystem::file_time_type>> ShaderDependenciesModifiedTime;

    /**
 * @brief 셰이더와 그 의존성 파일들의 최종 수정 시간을 기록합니다.
 * @param ShaderKey 셰이더를 식별하는 고유 키 (예: VertexKey 또는 PixelKey).
 * @param MainShaderFileName 컴파일된 주 셰이더 파일의 전체 경로.
 * @param Dependencies 컴파일 시 포함된 모든 의존성 파일 경로의 집합.
 */
    void RecordShaderDependencies(
        const std::wstring& ShaderKey,
        const std::wstring& MainShaderFileName,
        const std::set<std::wstring>& Dependencies);

    /**
    * @brief 주어진 셰이더 키에 연결된 의존성 파일들의 수정 여부를 확인합니다.
    * @param ShaderKey 확인할 셰이더의 키 (VertexKey 또는 PixelKey).
    * @return 의존성 파일 중 하나라도 수정되었거나 접근할 수 없으면 true, 그렇지 않으면 false.
    */
    bool CheckShaderModified(const std::wstring& ShaderKey) const;
    

    template<typename T>
    void SafeRelease(T*& comObject);

};

