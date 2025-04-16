#include "DXDShaderManager.h"
#include "Define.h"
#include "DXDInclude.h"
#include "UserInterface/Console.h"


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

    
    FDXDInclude includeManager(FileName);

    ID3DBlob* PsBlob = nullptr;
    ID3DBlob* ErrorBlob = nullptr;


    hr = D3DCompileFromFile(FileName.c_str(), Defines, &includeManager, EntryPoint.c_str(), "ps_5_0", shaderFlags, 0, &PsBlob, &ErrorBlob);
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

    
    const std::set<std::wstring>& dependencies = includeManager.GetIncludedFiles();

    // modified time 기록
    RecordShaderDependencies(Key, FileName, dependencies);

    
    
    //std::filesystem::path FilePath = FileName;
    //PixelShaderModifiedTime[NewPixelShader] = std::filesystem::last_write_time(FilePath);

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
    
    FDXDInclude includeManager(FileName);

    hr = D3DCompileFromFile(FileName.c_str(), Defines, &includeManager, EntryPoint.c_str(), "vs_5_0", 0, 0, &VertexShaderCSO, &ErrorBlob);
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

    // modified time 기록
    const std::set<std::wstring>& dependencies = includeManager.GetIncludedFiles();

    // modified time 기록
    RecordShaderDependencies(Key, FileName, dependencies);

    return S_OK;
}

HRESULT FDXDShaderManager::AddComputeShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines)
{
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;

    ID3DBlob* ComputeShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;
    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
//#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
//#endif
    FDXDInclude includeManager(FileName);

    hr = D3DCompileFromFile(FileName.c_str(), Defines, &includeManager, EntryPoint.c_str(), "cs_5_0", shaderFlags, 0, &ComputeShaderCSO, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            UE_LOG(LogLevel::Error, (char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11ComputeShader* NewComputeShader;
    hr = DXDDevice->CreateComputeShader(ComputeShaderCSO->GetBufferPointer(), ComputeShaderCSO->GetBufferSize(), nullptr, &NewComputeShader);
    if (FAILED(hr))
    {
        ComputeShaderCSO->Release();
        return hr;
    }

    ComputeShaders[Key] = NewComputeShader;

    ComputeShaderCSO->Release();

    // modified time 기록
    const std::set<std::wstring>& dependencies = includeManager.GetIncludedFiles();

    // modified time 기록
    RecordShaderDependencies(Key, FileName, dependencies);

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

    
    FDXDInclude includeManager(FileName);
    hr = D3DCompileFromFile(FileName.c_str(), Defines, &includeManager, EntryPoint.c_str(), "vs_5_0", shaderFlags, 0, &VertexShaderCSO, &ErrorBlob);
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

    ID3D11InputLayout* NewInputLayout = nullptr;
    if (Layout)
    {
        hr = DXDDevice->CreateInputLayout(Layout, LayoutSize, VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &NewInputLayout);
        if (FAILED(hr))
        {
            VertexShaderCSO->Release();
            return hr;
        }
    }
    VertexShaders[Key] = NewVertexShader;
    InputLayouts[Key] = NewInputLayout;

    VertexShaderCSO->Release();

    // modified time 기록
    const std::set<std::wstring>& dependencies = includeManager.GetIncludedFiles();

    // modified time 기록
    RecordShaderDependencies(Key, FileName, dependencies);

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

ID3D11ComputeShader* FDXDShaderManager::GetComputeShaderByKey(const std::wstring& Key) const
{
    if (ComputeShaders.Contains(Key))
    {
        return *ComputeShaders.Find(Key);
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

HRESULT FDXDShaderManager::ReloadComputeShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines)
{
    if (!ComputeShaders.Contains(Key))
    {
        UE_LOG(LogLevel::Warning, "Failed to reload compute shader : Key does not exist");
        return S_FALSE;
    }
    ID3D11ComputeShader* PreviousShader = ComputeShaders[Key];

    HRESULT hr = AddComputeShader(Key, FileName, EntryPoint, Defines);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "Failed to reload compute shader : Compilation Failed");
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
        UE_LOG(LogLevel::Display, "Successfully reloaded compute shader");
        return hr;
    }
}

HRESULT FDXDShaderManager::ReloadShaders(const std::wstring& VertexKey, const std::wstring& VertexFileName, const std::string& VertexEntryPoint,
    const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* VertexDefines,
    const std::wstring& PixelKey, const std::wstring& PixelFileName, const std::string& PixelEntryPoint, const D3D_SHADER_MACRO* PixelDefines)
{
    if (!VertexShaders.Contains(VertexKey) || !PixelShaders.Contains(PixelKey)/* || !InputLayouts.Contains(VertexKey)*/)
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

        UE_LOG(LogLevel::Warning, "Successfully compiled vertex shader, but failed to compile pixel shader. Reverting to previous vertex shader.");
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


HRESULT FDXDShaderManager::ReloadModifiedShaders(const std::wstring& VertexKey, const std::wstring& VertexFileName, const std::string& VertexEntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* VertexDefines, const std::wstring& PixelKey, const std::wstring& PixelFileName, const std::string& PixelEntryPoint, const D3D_SHADER_MACRO* PixelDefines)
{
    // input layout이 없을수도 있으니까
    if (!VertexShaders.Contains(VertexKey) || !PixelShaders.Contains(PixelKey)/* || !InputLayouts.Contains(VertexKey)*/)
    {
        UE_LOG(LogLevel::Warning, "Invalid Key : Parameter might be different with parameters at init time.");
        return S_FALSE;
    }
    // vertex pixel 모두 다 조사
    bool IsModified = false;
    {
        IsModified = CheckShaderModified(VertexKey);
    }
    if(!IsModified)
    {
        IsModified = CheckShaderModified(PixelKey);
    }
    if (IsModified)
    {
        UE_LOG(LogLevel::Display, "Shader file is modified. Recompiling shader.");
        return ReloadShaders(VertexKey, VertexFileName, VertexEntryPoint, Layout, LayoutSize, VertexDefines, PixelKey, PixelFileName, PixelEntryPoint, PixelDefines);
    }
    else
    {
        return S_FALSE;
    }
}

HRESULT FDXDShaderManager::ReloadModifiedComputeShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines)
{
    if (!ComputeShaders.Contains(Key))
    {
        UE_LOG(LogLevel::Warning, "Invalid Key : Parameter might be different with parameters at init time.");
        return S_FALSE;
    }
    // vertex pixel 모두 다 조사
    bool IsModified = false;
    {
        IsModified = CheckShaderModified(Key);
    }
    if (!IsModified)
    {
        IsModified = CheckShaderModified(Key);
    }
    if (IsModified)
    {
        UE_LOG(LogLevel::Display, "Shader file is modified. Recompiling shader.");
        return ReloadComputeShader(Key, FileName, EntryPoint, Defines);
    }
    else
    {
        return S_FALSE;
    }
}

void FDXDShaderManager::RecordShaderDependencies(const std::wstring& ShaderKey, const std::wstring& MainShaderFileName,
    const std::set<std::wstring>& Dependencies)
{
    TMap<std::wstring, std::filesystem::file_time_type> currentFileTimes;
    std::error_code ec; // 오류 코드를 받기 위한 변수

    // 1. 메인 셰이더 파일 시간 기록
    auto mainFileTime = std::filesystem::last_write_time(MainShaderFileName, ec);
    if (!ec) // 오류가 없으면
    {
        currentFileTimes[MainShaderFileName] = mainFileTime;
    }
    else
    {
        // 오류 발생 시 어떻게 처리할지 결정 (예: 맵에 추가하지 않거나, 특정 기본값 사용)
    }

    // 2. 의존성 파일 시간 기록
    for (const auto& dependencyPath : Dependencies)
    {
        ec.clear(); // 다음 호출을 위해 오류 코드 초기화
        auto dependencyFileTime = std::filesystem::last_write_time(dependencyPath, ec);
        if (!ec)
        {
            currentFileTimes[dependencyPath] = dependencyFileTime;
        }
        else
        {
            // 오류 처리
        }
    }

    // 3. 결과를 매니저의 주 의존성 맵에 저장 (기존 항목 덮어쓰기 또는 추가)
    ShaderDependenciesModifiedTime[ShaderKey] = currentFileTimes;
}

bool FDXDShaderManager::CheckShaderModified(const std::wstring& ShaderKey) const
{
    // 1. 저장된 의존성 정보 가져오기
    if (!ShaderDependenciesModifiedTime.Contains(ShaderKey))
    {
        // 일단 false 반환 (수정 안 됨)으로 처리.
        return false;
    }

    const TMap<std::wstring, std::filesystem::file_time_type>& storedDependencies = ShaderDependenciesModifiedTime[ShaderKey];

    if (storedDependencies.IsEmpty())
    {
        // 의존성 정보는 있으나 비어있는 경우 
        return false;
    }

    // 2. 각 의존성 파일의 현재 수정 시간 확인 및 비교
    std::error_code ec; // 오류 코드
    for (const auto& pair : storedDependencies)
    {
        const std::wstring& filePath = pair.Key;
        const std::filesystem::file_time_type storedTime = pair.Value;

        ec.clear(); // 오류 코드 초기화
        auto currentTime = std::filesystem::last_write_time(filePath, ec);

        if (ec) // 파일 접근 오류 (삭제되었거나 권한 없음 등)
        {
            return true; // 오류 발생 시 수정된 것으로 간주하여 리로드 유도
        }

        if (currentTime != storedTime) // 저장된 시간과 현재 시간이 다르면
        {
            return true; // 수정됨
        }
    }

    // 3. 모든 파일이 수정되지 않았음
    return false;
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
