#include "ConfigManager.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;
TMap<FName, FConfigCacheIni> ConfigManager::ConfigCache;

void ConfigManager::GetConfigValue(const FString& SectionName, const FString& Key, FString& OutValue, const FName& IniName)
{
    if (FConfigCacheIni* Cache = ConfigCache.Find(IniName))
    {
        if (const TMap<FString, FString>* Section = Cache->CacheConfig.Find(SectionName))
        {
            if (const FString* Value = Section->Find(Key))
            {
                OutValue = *Value;
                return;
            }
        }
    }
    OutValue = ""; // 값이 없으면 빈 문자열
}

void ConfigManager::SetConfigValue(const FString& SectionName, const FString& Key, const FString& Value, const FName& IniName)
{
    FConfigCacheIni* Cache = ConfigCache.Find(IniName);
    if (!Cache)
    {
        // 새로운 INI 캐시 추가
        FConfigCacheIni NewCache;
        NewCache.ConfigPath = "./Config/Default.ini";
        NewCache.ConfigName = IniName.ToString();
        ConfigCache.Add(IniName, NewCache);

        SetConfigValue(SectionName, Key, Value, IniName);
        return;
    }

    // 섹션이 없으면 생성
    if (!Cache->CacheConfig.Contains(SectionName))
    {
        Cache->CacheConfig.Add(SectionName, TMap<FString, FString>());
    }

    // 키-값 설정
    Cache->CacheConfig[SectionName][Key] = Value;
}

void ConfigManager::SaveAllConfigs()
{
    for (const auto& CachePair : ConfigCache)
    {
        WriteIniFile(CachePair.Value.ConfigPath , CachePair.Value);
    }
}

void ConfigManager::LoadAllConfigs()
{
    // Clear cache
    ConfigCache.Empty();

    // .ini file loads from the directory defined in the macro.
#define LOAD_CONFIG_FILES(path) LoadIniFromDirectory(path);
    CONFIG_PATHS(LOAD_CONFIG_FILES)
#undef LOAD_CONFIG_FILES
}

TMap<FString, TMap<FString, FString>> ConfigManager::ReadIniFile(const FString& FilePath)
{
    TMap<FString, TMap<FString, FString>> Config;
    std::ifstream File(*FilePath);
    std::string Line;
    // 섹션이 정의되지 않은 경우 기본 섹션 이름 지정
    FString CurrentSection = TEXT("Default");

    while (std::getline(File, Line))
    {
        // 빈 줄은 무시
        if (Line.empty())
        {
            continue;
        }
        // 세미콜론으로 시작하는 주석 라인 무시
        if (Line[0] == ';')
        {
            continue;
        }
        // '['로 시작하면 섹션 헤더로 판단
        if (Line[0] == '[')
        {
            size_t EndPos = Line.find(']');
            if (EndPos != std::string::npos)
            {
                // 섹션 이름 추출 (대괄호 사이의 문자열)
                CurrentSection = FString(Line.substr(1, EndPos - 1).c_str());
            }
            continue;
        }
        
        // 'key=value' 형식의 라인 파싱
        std::istringstream SS(Line);
        std::string Key, Value;
        if (std::getline(SS, Key, '=') && std::getline(SS, Value))
        {
            FString KeyStr(Key.c_str());
            FString ValueStr(Value.c_str());
            
            // 현재 섹션에 해당하는 내부 맵이 없으면 생성
            if (!Config.Contains(CurrentSection))
            {
                Config.Add(CurrentSection, TMap<FString, FString>());
            }
            // 해당 섹션에 키–값 쌍 저장
            Config[CurrentSection].Add(KeyStr, ValueStr);
        }
    }
    
    return Config;
}

void ConfigManager::WriteIniFile(const FString& FilePath, const FConfigCacheIni& Cache)
{
    std::ofstream File(GetData(FilePath));
    
    for (const auto& SectionPair : Cache.CacheConfig)
    {
        File << "[" << *SectionPair.Key << "]\n";
        for (const auto& KeyValuePair : SectionPair.Value)
        {
            File << *KeyValuePair.Key << "=" << *KeyValuePair.Value << "\n";
        }
        File << "\n";
    }
    File.close();
}

void ConfigManager::LoadIniFromDirectory(const FString& Directory)
{
    try
    {
        for (const auto& Entry : fs::recursive_directory_iterator(GetData(Directory)))
        {
            if (Entry.is_regular_file() && Entry.path().extension() == ".ini")
            {
                FString FileName = Entry.path().filename().string();
                const FString FilePath = Entry.path().string();
                FName IniName = FileName; // 파일 이름을 IniName으로 사용 "editor.ini"
                FConfigCacheIni Cache;
                Cache.ConfigName = FileName;
                Cache.ConfigPath = FilePath;
                Cache.CacheConfig = ReadIniFile(FilePath);
                ConfigCache.Add(IniName, Cache);
                UE_LOG(LogLevel::Display, "INI FILE LOADED : %s", *FileName);
            }
        }
    }
    catch (const fs::filesystem_error& e)
    {
        UE_LOG(LogLevel::Error, "INI FILE LOAD FAILED : %s", e.what());
    }
}
