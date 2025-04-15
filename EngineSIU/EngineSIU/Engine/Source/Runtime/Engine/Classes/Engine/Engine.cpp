#include "Engine.h"

#include "ConfigManager.h"
#include "EditorEngine.h"
#include "UnrealEd/SceneManager.h"
#include "UObject/Casts.h"
#include "World/World.h"

UEngine* GEngine = nullptr;

void UEngine::Init()
{
}

FWorldContext* UEngine::GetWorldContextFromWorld(const UWorld* InWorld)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->World() == InWorld)
        {
            return WorldContext;
        }
    }
    return nullptr;
}

FWorldContext& UEngine::CreateNewWorldContext(EWorldType InWorldType)
{
    FWorldContext* NewWorldContext = new FWorldContext();
    WorldList.Add(NewWorldContext);
    NewWorldContext->WorldType = InWorldType;
    NewWorldContext->ContextHandle = FName(*FString::Printf(TEXT("WorldContext_%d"), NextWorldContextHandle++));

    return *NewWorldContext;
}


void UEngine::LoadLevel(const FString& FileName)
{
    SceneManager::LoadSceneFromJsonFile(FileName, *ActiveWorld);
}

void UEngine::SaveLevel(const FString& FileName)
{
    SceneManager::SaveSceneToJsonFile(FileName, *ActiveWorld);
}

void UEngine::LoadConfig()
{
    ConfigManager::LoadAllConfigs();
}

void UEngine::SaveConfig()
{
    ConfigManager::SaveAllConfigs();
}
