#pragma once

#include <sstream>

#include "World/World.h"
#include "Math/Vector.h"

// 매크로로 경로 정의
#define CONFIG_PATHS(F) \
F("./Config") \


struct FConfigCacheIni
{
    FString ConfigName;
    FString ConfigPath;

    // <Section, <Key, Value>>
    TMap<FString, TMap<FString, FString>> CacheConfig;
};


class ConfigManager
{
public:
    static void GetConfigValue(const FString& SectionName, const FString& Key, FString& OutValue, const FName& IniName);
    static void SetConfigValue(const FString& SectionName, const FString& Key, const FString& Value, const FName& IniName);

    static void SaveAllConfigs();
    static void LoadAllConfigs();

private:
    static TMap<FString, TMap<FString, FString>> ReadIniFile(const FString& FilePath);
    static void WriteIniFile(const FString& FilePath, const FConfigCacheIni& Cache);

    static void LoadIniFromDirectory(const FString& Directory);
    
private:
    static TMap<FName, FConfigCacheIni> ConfigCache;
};
