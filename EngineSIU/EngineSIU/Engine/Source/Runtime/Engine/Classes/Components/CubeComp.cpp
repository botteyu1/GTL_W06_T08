#include "CubeComp.h"
#include "Engine/FLoaderOBJ.h"
#include "UObject/ObjectFactory.h"


UCubeComp::UCubeComp()
{
    SetType(StaticClass()->GetName());
    LocalBoundingBox.max = { 1,1,1 };
    LocalBoundingBox.min = { -1,-1,-1 };

}

void UCubeComp::InitializeComponent()
{
    Super::InitializeComponent();

    FManagerOBJ::CreateStaticMesh("Assets/helloBlender.obj");
    SetStaticMesh(FManagerOBJ::GetStaticMesh(L"helloBlender.obj"));
}

void UCubeComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}