#include "DXDShaderManager.h"
#include "Define.h"


FDXDShaderManager::FDXDShaderManager(ID3D11Device* Device)
    : DXDDevice(Device)
{
    VertexShaders.Empty();
    PixelShaders.Empty();
}

void FDXDShaderManager::ReleaseAllShader()
{
    for (auto& [Key, Shader] : VertexShaders)
    {
        if (Shader)
        {
            Shader->Release();
            Shader = nullptr;
        }
    }
    VertexShaders.Empty();

    for (auto& [Key, Shader] : PixelShaders)
    {
        if (Shader)
        {
            Shader->Release();
            Shader = nullptr;
        }
    }
    PixelShaders.Empty();

}

HRESULT FDXDShaderManager::AddPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines)
{
    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    HRESULT hr = S_OK;

    if (DXDDevice == nullptr)
        return S_FALSE;

    ID3DBlob* PsBlob = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    hr = D3DCompileFromFile(FileName.c_str(), Defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), "ps_5_0", shaderFlags, 0, &PsBlob, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            UE_LOG(LogLevel::Error, (char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11PixelShader* NewPixelShader;
    hr = DXDDevice->CreatePixelShader(PsBlob->GetBufferPointer(), PsBlob->GetBufferSize(), nullptr, &NewPixelShader);
    if (PsBlob)
    {
        PsBlob->Release();
    }
    if (FAILED(hr))
        return hr;

    PixelShaders[Key] = NewPixelShader;

    return S_OK;
}

HRESULT FDXDShaderManager::AddVertexShader(const std::wstring& Key, const std::wstring& FileName)
{
    return E_NOTIMPL;
}

HRESULT FDXDShaderManager::AddVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines)
{
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;

    ID3DBlob* VertexShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    hr = D3DCompileFromFile(FileName.c_str(), Defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), "vs_5_0", 0, 0, &VertexShaderCSO, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            UE_LOG(LogLevel::Error, (char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    hr = DXDDevice->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        VertexShaderCSO->Release();
        return hr;
    }

    VertexShaders[Key] = NewVertexShader;

    VertexShaderCSO->Release();

    return S_OK;
}

HRESULT FDXDShaderManager::AddInputLayout(const std::wstring& Key, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize)
{
    return S_OK;
}

void FDXDShaderManager::AddVertexShader(const std::wstring& Key, ID3D11VertexShader* VerteShader)
{
    if (!VerteShader)
    {
        return;
    }
    VertexShaders[Key] = VerteShader;
}

void FDXDShaderManager::AddVertexShader(const std::wstring& Key, ID3D11PixelShader* PixelShader)
{
    if (!PixelShader)
    {
        return;
    }
    PixelShaders[Key] = PixelShader;
}

void FDXDShaderManager::AddInputLayout(const std::wstring& Key, ID3D11InputLayout* InputLayout)
{
    if (!InputLayout)
    {
        return;
    }
    InputLayouts[Key] = InputLayout;
}

HRESULT FDXDShaderManager::AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* Defines)
{
    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;

    ID3DBlob* VertexShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    hr = D3DCompileFromFile(FileName.c_str(), Defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), "vs_5_0", shaderFlags, 0, &VertexShaderCSO, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            UE_LOG(LogLevel::Error, (char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    hr = DXDDevice->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        return hr;
    }

    ID3D11InputLayout* NewInputLayout;
    hr = DXDDevice->CreateInputLayout(Layout, LayoutSize, VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &NewInputLayout);
    if (FAILED(hr))
    {
        VertexShaderCSO->Release();
        return hr;
    }

    VertexShaders[Key] = NewVertexShader;
    InputLayouts[Key] = NewInputLayout;

    VertexShaderCSO->Release();

    return S_OK;
}

ID3D11InputLayout* FDXDShaderManager::GetInputLayoutByKey(const std::wstring& Key) const
{
    if (InputLayouts.Contains(Key))
    {
        return *InputLayouts.Find(Key);
    }
    return nullptr;
}

ID3D11VertexShader* FDXDShaderManager::GetVertexShaderByKey(const std::wstring& Key) const
{
    if (VertexShaders.Contains(Key))
    {
        return *VertexShaders.Find(Key);
    }
    return nullptr;
}

ID3D11PixelShader* FDXDShaderManager::GetPixelShaderByKey(const std::wstring& Key) const
{
    if (PixelShaders.Contains(Key))
    {
        return *PixelShaders.Find(Key);
    }
    return nullptr;
}

void FDXDShaderManager::RemoveInputLayoutByKey(const std::wstring& Key)
{
    if (InputLayouts.Contains(Key))
    {
        InputLayouts[Key]->Release();
        InputLayouts[Key] = nullptr;
        InputLayouts.Remove(Key);
    }
}

void FDXDShaderManager::RemoveVertexShaderByKey(const std::wstring& Key)
{
    if (VertexShaders.Contains(Key))
    {
        VertexShaders[Key]->Release();
        VertexShaders[Key] = nullptr;
        VertexShaders.Remove(Key);
    }
}

void FDXDShaderManager::RemovePixelShaderByKey(const std::wstring& Key)
{
    if (PixelShaders.Contains(Key))
    {
        PixelShaders[Key]->Release();
        PixelShaders[Key] = nullptr;
        PixelShaders.Remove(Key);
    }
}

void FDXDShaderManager::SetVertexShader(const std::wstring& KeyName, ID3D11DeviceContext* DeviceContext) const
{
    if (DeviceContext == nullptr)
    {
        return;
    }
    if (ID3D11VertexShader* Shader = GetVertexShaderByKey(KeyName))
    {
        DeviceContext->VSSetShader(Shader, nullptr, 0);
    }
}

void FDXDShaderManager::SetVertexShaderAndInputLayout(const std::wstring KeyName, ID3D11DeviceContext* DeviceContext) const
{
    if (DeviceContext == nullptr)
    {
        return;
    }
    if (ID3D11VertexShader* Shader = GetVertexShaderByKey(KeyName))
    {
            DeviceContext->VSSetShader(Shader, nullptr, 0);
    }
    if (ID3D11InputLayout* Layout = GetInputLayoutByKey(KeyName))
    {
        DeviceContext->IASetInputLayout(Layout);
    }
}

void FDXDShaderManager::SetPixelShader(const std::wstring& KeyName, ID3D11DeviceContext* DeviceContext) const
{
    if (DeviceContext == nullptr)
    {
        return;
    }
    if (ID3D11PixelShader* Shader = GetPixelShaderByKey(KeyName))
    {
        DeviceContext->PSSetShader(Shader, nullptr, 0);
    }
}

void FDXDShaderManager::SetInputLayout(const std::wstring& KeyName, ID3D11DeviceContext* DeviceContext) const
{
    if (DeviceContext == nullptr)
    {
        return;
    }
    if (ID3D11InputLayout* Layout = GetInputLayoutByKey(KeyName))
    {
        DeviceContext->IASetInputLayout(Layout);
    }
}


HRESULT FDXDShaderManager::ReloadVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines)
{
    if (!VertexShaders.Contains(Key))
    {
        UE_LOG(LogLevel::Warning, "Failed to reload vertex shader : Key does not exist");
        return S_FALSE;
    }
    ID3D11VertexShader* PreviousShader = VertexShaders[Key];

    HRESULT hr = AddVertexShader(Key, FileName, EntryPoint, Defines);
    if(FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "Failed to reload vertex shader : Compilation Failed");
        return hr;
    }
    else
    {
        // 버텍스 셰이더 컴파일에 성공했고, VertexShaders[Key]에는 새로운 shadercode가 들어가있음
        // 원래 있었던 코드를 release
        if (PreviousShader)
        {
            PreviousShader->Release();
        }
        UE_LOG(LogLevel::Display, "Successfully reloaded vertex shader");
        return hr;
    }
}

HRESULT FDXDShaderManager::ReloadPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines)
{
    if (!PixelShaders.Contains(Key))
    {
        UE_LOG(LogLevel::Warning, "Failed to reload pixel shader : Key does not exist");
        return S_FALSE;
    }
    ID3D11PixelShader* PreviousShader = PixelShaders[Key];

    HRESULT hr = AddPixelShader(Key, FileName, EntryPoint, Defines);
    if(FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "Failed to reload pixel shader : Compilation Failed");
        return hr;
    }
    else
    {
        // 버텍스 셰이더 컴파일에 성공했고, PixelShaders[Key]에는 새로운 shadercode가 들어가있음
        // 원래 있었던 코드를 release
        if (PreviousShader)
        {
            PreviousShader->Release();
        }
        UE_LOG(LogLevel::Display, "Successfully reloaded pixel shader");
        return hr;
    }
}

HRESULT FDXDShaderManager::ReloadShaders(const std::wstring& VertexKey, const std::wstring& VertexFileName, const std::string& VertexEntryPoint,
    const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* VertexDefines,
    const std::wstring& PixelKey, const std::wstring& PixelFileName, const std::string& PixelEntryPoint, const D3D_SHADER_MACRO* PixelDefines)
{
    if (!VertexShaders.Contains(VertexKey) || !PixelShaders.Contains(PixelKey) || !InputLayouts.Contains(VertexKey))
    {
        UE_LOG(LogLevel::Warning, "Invalid Key : Adding new shaders. This happens when shader compilation at intialization failed.");
    }
    //  실패 시 revert할 shader
    ID3D11VertexShader* PreviousVertexShader = VertexShaders[VertexKey];
    ID3D11InputLayout* PreviousInputLayout = InputLayouts[VertexKey];
    ID3D11PixelShader* PreviousPixelShader = PixelShaders[PixelKey];

    HRESULT hr = AddVertexShaderAndInputLayout(VertexKey, VertexFileName, VertexEntryPoint, Layout, LayoutSize, VertexDefines);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "Failed to reload vertex shader : Compilation Failed");
        return hr;
    }

    hr = AddPixelShader(PixelKey, PixelFileName, PixelEntryPoint, PixelDefines);
    if (FAILED(hr))
    {
        // vertex shader를 되돌림
        // 새로 만든 shader를 release하고 이전의 shader로 돌아감
        SafeRelease(VertexShaders[VertexKey]);
        SafeRelease(InputLayouts[VertexKey]);

        VertexShaders[VertexKey] = PreviousVertexShader;
        InputLayouts[VertexKey] = PreviousInputLayout;

        UE_LOG(LogLevel::Warning, "Successfully compiled vertex shader, but failed to compile pixel shader. Reverting previous vertex shader.");
        return hr;
    }
    else
    {
        // 이전의 포인터는 release
        SafeRelease(PreviousVertexShader);
        SafeRelease(PreviousInputLayout);
        SafeRelease(PreviousPixelShader);
        UE_LOG(LogLevel::Display, "Successfully reloaded vertex / pixel shader");
        return hr;
    }
}

template<typename T>
void FDXDShaderManager::SafeRelease(T*& comObject)
{
    if (comObject)
    {
        comObject->Release();
        comObject = nullptr;
    }
}
