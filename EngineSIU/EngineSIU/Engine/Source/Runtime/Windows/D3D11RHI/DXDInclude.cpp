#include "DXDInclude.h"

#include <fstream>

FDXDInclude::FDXDInclude(const std::wstring& InBaseDirectory): m_BaseDirectory(std::filesystem::absolute(InBaseDirectory))
{
    if (!std::filesystem::is_directory(m_BaseDirectory))
    {
        // 디렉토리 대신 파일이 주어졌을 경우 부모 경로 사용 시도
        if (std::filesystem::exists(InBaseDirectory))
        {
            m_BaseDirectory = std::filesystem::absolute(InBaseDirectory).parent_path();
        }
        else
        {
        }
    }
    // 기본 디렉토리 자체를 잠재적 검색 위치로 추가
    AddSearchDirectory(m_BaseDirectory);
}

void FDXDInclude::AddSearchDirectory(const std::wstring& DirectoryPath)
{
    std::filesystem::path path = std::filesystem::absolute(DirectoryPath);
    if (std::filesystem::is_directory(path))
    {
        m_SearchDirectories.push_back(path);
        //UE_LOG(LogLevel::Display, L"검색 디렉토리 추가됨: %ls", path.c_str());
    }
    else
    {
        //UE_LOG(LogLevel::Warning, L"검색 디렉토리 추가 실패 (디렉토리가 아니거나 존재하지 않음): %ls", path.c_str());
    }
}

HRESULT FDXDInclude::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
{
    // 사용되지 않는 매개변수 경고 방지
    (void)pParentData;

    std::filesystem::path requestedPath(pFileName); // char*를 경로 객체로 변환
    std::filesystem::path fullPath;
    bool bFound = false;

    // 1. 파일 찾기 시도
    if (IncludeType == D3D_INCLUDE_LOCAL) // #include "..." (로컬 포함)
    {
        // 먼저 기본 디렉토리 기준으로 상대 경로 시도
        fullPath = m_BaseDirectory / requestedPath;
        std::error_code ec;
        if (std::filesystem::exists(fullPath, ec) && !std::filesystem::is_directory(fullPath, ec)) // 파일 존재 및 디렉토리 아닌지 확인
        {
            bFound = true;
        }
    }

    // 2. 로컬에서 찾지 못했거나 시스템 포함인 경우, 등록된 디렉토리 검색
    if (!bFound) // D3D_INCLUDE_SYSTEM (#include <...>) (시스템 포함)도 처리
    {
        for (const auto& dir : m_SearchDirectories)
        {
            fullPath = dir / requestedPath;
            std::error_code ec;
            if (std::filesystem::exists(fullPath, ec) && !std::filesystem::is_directory(fullPath, ec))
            {
                bFound = true;
                break; // 찾음
            }
        }
    }

    wchar_t const* LL;
    // 3. 파일을 찾았다면, 내용을 읽습니다.
    if (bFound)
    {
        try
        {
            // 정규 경로(canonical path) 얻기 (절대 경로, 심볼릭 링크 해결, '.'/'..' 제거)
            std::error_code ec;
            fullPath = std::filesystem::canonical(fullPath, ec);
            if (ec) {
                // canonical 실패 시 절대 경로로 대체 (예: 가끔 네트워크 경로)
                fullPath = std::filesystem::absolute(fullPath);
                // UE_LOG(LogLevel::Warning, L"정규 경로 얻기 실패 %ls, 절대 경로 사용: %ls. 오류: %hs",
                //        requestedPath.c_str(), fullPath.c_str(), ec.message().c_str());
            }


            // 파일 내용 읽기
            std::ifstream fileStream(fullPath, std::ios::binary | std::ios::ate); // 바이너리 모드 | 끝에서 열기 (크기 얻기 위해)
            if (!fileStream.is_open())
            {
                //UE_LOG(LogLevel::Error, L"포함 오류: 찾은 파일에 대한 파일 스트림 열기 실패: %ls", fullPath.c_str());
                return E_FAIL;
            }

            std::streamsize size = fileStream.tellg(); // 파일 크기 얻기
            if (size <= 0) {
                // 비어 있거나 유효하지 않은 파일 처리
                char* buffer = new char[1]; // 최소 버퍼 할당
                buffer[0] = '\0';
                *ppData = buffer;
                *pBytes = 0; // 크기는 0
                //UE_LOG(LogLevel::Warning, L"포함 경고: 파일이 비어 있거나 크기 얻기 실패: %ls", fullPath.c_str());
                // 비어 있어도 의존성은 기록
                m_IncludedFiles.insert(fullPath.wstring());
                return S_OK;
            }

            fileStream.seekg(0, std::ios::beg); // 시작 지점으로 이동

            // 버퍼 할당 (+1은 잠재적 null 종결자용, D3DCompile에 엄격히 요구되지는 않음)
            char* buffer = new char[static_cast<size_t>(size) + 1];
            if (!fileStream.read(buffer, size))
            {
                //UE_LOG(LogLevel::Error, L"포함 오류: 파일 내용 읽기 실패: %ls", fullPath.c_str());
                delete[] buffer; // 할당된 메모리 정리
                return E_FAIL;
            }
            buffer[size] = 0; // 안전을 위해 Null 종결 (선택 사항)


            // 성공 - 데이터와 크기 반환
            *ppData = buffer;
            *pBytes = static_cast<UINT>(size);

            // 이 파일을 의존성으로 기록
            m_IncludedFiles.insert(fullPath.wstring());
            // UE_LOG(LogLevel::Display, L"포함 성공: %ls 열림", fullPath.c_str()); // 선택 사항: 성공한 포함 로그
            return S_OK;
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            //UE_LOG(LogLevel::Error, L"포함 파일 시스템 오류: %hs (%ls 접근 중)", e.what(), fullPath.c_str());
            return E_FAIL; // 실패 반환
        }
        catch (const std::bad_alloc&)
        {
            //UE_LOG(LogLevel::Error, L"포함 메모리 오류: %ls 버퍼 할당 실패", fullPath.c_str());
            return E_OUTOFMEMORY;
        }
        catch (...)
        {
            //UE_LOG(LogLevel::Error, L"포함 오류: %ls 열기/읽기 중 알 수 없는 예외 발생", fullPath.c_str());
            return E_FAIL;
        }
    }
    else
    {
        // 어떤 검색 경로에서도 파일을 찾지 못함
        //UE_LOG(LogLevel::Error, L"포함 오류: 파일을 찾을 수 없음: %hs (기준: %ls)", pFileName, m_BaseDirectory.c_str());
        // 시도한 검색 경로 로그?
        // for (const auto& dir : m_SearchDirectories) { UE_LOG(LogLevel::Display, L"검색 경로: %ls", dir.c_str()); }
        return E_FAIL; // 실패 반환: 파일 찾지 못함
    }
}

HRESULT FDXDInclude::Close(LPCVOID pData)
{
    // 데이터 포인터를 원래 타입으로 캐스팅하여 해제합니다.
    char* buffer = static_cast<char*>(const_cast<void*>(pData));
    delete[] buffer;
    return S_OK;
}
