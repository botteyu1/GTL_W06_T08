#pragma once // 헤더 파일 중복 포함 방지

#include <d3d11.h>
#include <d3dcompiler.h> // ID3DInclude, D3D_INCLUDE_TYPE에 필요
#include <vector>
#include <string>
#include <set>
#include <filesystem> // C++17 필요



/**
 * @brief ID3DInclude를 구현하여 셰이더 포함(include)을 처리하고 의존성을 추적합니다.
 *
 * 이 클래스는 D3DCompileFromFile과 함께 사용되어 다음을 수행합니다:
 * 1. #include된 셰이더 파일(.hlsli 등)을 찾아 엽니다.
 * 2. 컴파일 중 성공적으로 열린 모든 파일의 기록을 유지합니다.
 */
class FDXDInclude : public ID3DInclude
{
public:
    /**
     * @brief 생성자.
     * @param InBaseDirectory 컴파일 중인 주 셰이더 파일이 있는 디렉토리. 로컬 포함(#include "...")의 기본 검색 경로로 사용됩니다.
     */
    FDXDInclude(const std::wstring& InBaseDirectory);

    // --- 공개 메서드 ---

    /**
     * @brief 시스템 포함(#include <...>)의 검색 경로 목록에 디렉토리를 추가합니다.
     * 또한 로컬 포함 파일이 기본 디렉토리 기준으로 찾아지지 않았을 때의 대체 경로로도 사용됩니다.
     * @param DirectoryPath 디렉토리의 절대 또는 상대 경로.
     */
    void AddSearchDirectory(const std::wstring& DirectoryPath);

    /**
     * @brief 컴파일 과정 중 성공적으로 포함된 전체 파일 경로 집합을 반환합니다.
     * @return 포함된 파일 경로 집합에 대한 상수 참조.
     */
    const std::set<std::wstring>& GetIncludedFiles() const
    {
        return m_IncludedFiles;
    }

    // --- ID3DInclude 구현 ---

    /**
     * @brief D3DCompile이 #include 지시문을 만났을 때 호출됩니다.
     */
    HRESULT __stdcall Open(
        D3D_INCLUDE_TYPE IncludeType,    // 포함 유형 (Local 또는 System)
        LPCSTR pFileName,                // #include 지시문에 명시된 파일명
        LPCVOID pParentData,             // 부모 파일의 내용 (종종 NULL)
        LPCVOID* ppData,                 // [out] 포함된 파일 내용을 담을 버퍼 포인터
        UINT* pBytes) override           // [out] 버퍼 크기 (바이트)
    ;

    /**
     * @brief D3DCompile이 Open()에서 할당된 메모리를 해제하기 위해 호출합니다.
     */
    HRESULT __stdcall Close(LPCVOID pData) override;

private:
    std::filesystem::path m_BaseDirectory;                  // 컴파일 중인 루트 셰이더 파일의 디렉토리
    std::vector<std::filesystem::path> m_SearchDirectories; // 포함 파일을 검색할 추가 디렉토리 목록
    std::set<std::wstring> m_IncludedFiles;                 // 성공적으로 열린 모든 파일의 정규화된 경로를 저장
};
