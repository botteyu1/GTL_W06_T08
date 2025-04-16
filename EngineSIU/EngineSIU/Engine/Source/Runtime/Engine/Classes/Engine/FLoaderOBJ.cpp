#include "FLoaderOBJ.h"

#include "UObject/ObjectFactory.h"
#include "Components/Material/Material.h"
#include "Components/Mesh/StaticMesh.h"

#include <fstream>
#include <sstream>

bool FLoaderOBJ::ParseOBJ(const FString& ObjFilePath, FObjInfo& OutObjInfo)
{
    std::ifstream OBJ(ObjFilePath.ToWideString());
    if (!OBJ)
    {
        return false;
    }

    OutObjInfo.FilePath = ObjFilePath.ToWideString().substr(0, ObjFilePath.ToWideString().find_last_of(L"\\/") + 1);
    OutObjInfo.ObjectName = ObjFilePath.ToWideString();
    // ObjectName은 wstring 타입이므로, 이를 string으로 변환 (간단한 ASCII 변환의 경우)
    std::wstring wideName = OutObjInfo.ObjectName.substr(ObjFilePath.ToWideString().find_last_of(L"\\/") + 1);;
    std::string fileName(wideName.begin(), wideName.end());

    // 마지막 '.'을 찾아 확장자를 제거
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        OutObjInfo.DisplayName = fileName.substr(0, dotPos);
    }
    else
    {
        OutObjInfo.DisplayName = fileName;
    }

    /**
     * 블렌더 Export 설정
     *   - General
     *       Forward Axis:  Y
     *       Up Axis:       Z
     *   - Geometry
     *       ✅ Triangulated Mesh
     */

    std::string Line;

    while (std::getline(OBJ, Line))
    {
        if (Line.empty() || Line[0] == '#')
            continue;

        std::istringstream LineStream(Line);
        std::string Token;
        LineStream >> Token;

        if (Token == "mtllib")
        {
            LineStream >> Line;
            OutObjInfo.MatName = Line;
            continue;
        }

        if (Token == "usemtl")
        {
            LineStream >> Line;
            FString MatName(Line);

            if (!OutObjInfo.MaterialSubsets.IsEmpty())
            {
                FMaterialSubset& LastSubset = OutObjInfo.MaterialSubsets[OutObjInfo.MaterialSubsets.Num() - 1];
                LastSubset.IndexCount = OutObjInfo.VertexIndices.Num() - LastSubset.IndexStart;
            }

            FMaterialSubset MaterialSubset;
            MaterialSubset.MaterialName = MatName;
            MaterialSubset.IndexStart = OutObjInfo.VertexIndices.Num();
            MaterialSubset.IndexCount = 0;
            OutObjInfo.MaterialSubsets.Add(MaterialSubset);
        }

        if (Token == "g" || Token == "o")
        {
            LineStream >> Line;
            OutObjInfo.GroupName.Add(Line);
            OutObjInfo.NumOfGroup++;
        }

        if (Token == "v") // Vertex
        {
            float X, Y, Z;
            LineStream >> X >> Y >> Z;
            OutObjInfo.Vertices.Add(FVector(X, Y * -1.f, Z));
            continue;
        }

        if (Token == "vn") // Normal
        {
            float NormalX, NormalY, NormalZ;
            LineStream >> NormalX >> NormalY >> NormalZ;
            OutObjInfo.Normals.Add(FVector(NormalX, NormalY * -1, NormalZ));
            continue;
        }

        if (Token == "vt") // Texture
        {
            float U, V;
            LineStream >> U >> V;
            OutObjInfo.UVs.Add(FVector2D(U, 1.f - V));
            continue;
        }

        if (Token == "f")
        {
            TArray<uint32> FaceVertexIndices;  // 이번 페이스의 정점 인덱스
            TArray<uint32> FaceNormalIndices;  // 이번 페이스의 법선 인덱스
            TArray<uint32> FaceUVIndices; // 이번 페이스의 텍스처 인덱스

            while (LineStream >> Token)
            {
                std::istringstream TokenStream(Token);
                std::string Part;
                TArray<std::string> FacePieces;

                uint32 vertexIndex = 0;
                uint32 textureIndex = UINT32_MAX;
                uint32 normalIndex = UINT32_MAX;

                // v
                if (std::getline(TokenStream, Part, '/'))
                {
                    if (!Part.empty())
                    {
                        vertexIndex = std::stoi(Part) - 1;
                    }
                }

                // vt
                if (std::getline(TokenStream, Part, '/'))
                {
                    if (!Part.empty())
                    {
                        textureIndex = std::stoi(Part) - 1;
                    }
                }

                // vn
                if (std::getline(TokenStream, Part, '/'))
                {
                    if (!Part.empty())
                    {
                        normalIndex = std::stoi(Part) - 1;
                    }
                }

                FaceVertexIndices.Add(vertexIndex);
                FaceUVIndices.Add(textureIndex);
                FaceNormalIndices.Add(normalIndex);
            }

            if (FaceVertexIndices.Num() == 3) // 삼각형
            {
                // 반시계 방향(오른손 좌표계)을 시계 방향(왼손 좌표계)으로 변환: 0-2-1
                OutObjInfo.VertexIndices.Add(FaceVertexIndices[0]);
                OutObjInfo.VertexIndices.Add(FaceVertexIndices[2]);
                OutObjInfo.VertexIndices.Add(FaceVertexIndices[1]);

                OutObjInfo.UVIndices.Add(FaceUVIndices[0]);
                OutObjInfo.UVIndices.Add(FaceUVIndices[2]);
                OutObjInfo.UVIndices.Add(FaceUVIndices[1]);

                OutObjInfo.NormalIndices.Add(FaceNormalIndices[0]);
                OutObjInfo.NormalIndices.Add(FaceNormalIndices[2]);
                OutObjInfo.NormalIndices.Add(FaceNormalIndices[1]);
            }
            else if (FaceVertexIndices.Num() == 4) // 쿼드
            {
                // 첫 번째 삼각형: 0-2-1
                OutObjInfo.VertexIndices.Add(FaceVertexIndices[0]);
                OutObjInfo.VertexIndices.Add(FaceVertexIndices[2]);
                OutObjInfo.VertexIndices.Add(FaceVertexIndices[1]);

                OutObjInfo.UVIndices.Add(FaceUVIndices[0]);
                OutObjInfo.UVIndices.Add(FaceUVIndices[2]);
                OutObjInfo.UVIndices.Add(FaceUVIndices[1]);

                OutObjInfo.NormalIndices.Add(FaceNormalIndices[0]);
                OutObjInfo.NormalIndices.Add(FaceNormalIndices[2]);
                OutObjInfo.NormalIndices.Add(FaceNormalIndices[1]);

                // 두 번째 삼각형: 0-3-2
                OutObjInfo.VertexIndices.Add(FaceVertexIndices[0]);
                OutObjInfo.VertexIndices.Add(FaceVertexIndices[3]);
                OutObjInfo.VertexIndices.Add(FaceVertexIndices[2]);

                OutObjInfo.UVIndices.Add(FaceUVIndices[0]);
                OutObjInfo.UVIndices.Add(FaceUVIndices[3]);
                OutObjInfo.UVIndices.Add(FaceUVIndices[2]);

                OutObjInfo.NormalIndices.Add(FaceNormalIndices[0]);
                OutObjInfo.NormalIndices.Add(FaceNormalIndices[3]);
                OutObjInfo.NormalIndices.Add(FaceNormalIndices[2]);
            }
        }
    }

    if (!OutObjInfo.MaterialSubsets.IsEmpty())
    {
        FMaterialSubset& LastSubset = OutObjInfo.MaterialSubsets[OutObjInfo.MaterialSubsets.Num() - 1];
        LastSubset.IndexCount = OutObjInfo.VertexIndices.Num() - LastSubset.IndexStart;
    }

    return true;
}

bool FLoaderOBJ::ParseMaterial(FObjInfo& OutObjInfo, OBJ::FStaticMeshRenderData& OutFStaticMesh)
{
    // Subset
    OutFStaticMesh.MaterialSubsets = OutObjInfo.MaterialSubsets;

    std::ifstream MtlFile(OutObjInfo.FilePath + OutObjInfo.MatName.ToWideString());
    if (!MtlFile.is_open())
    {
        return false;
    }

    std::string Line;
    int32 MaterialIndex = -1;

    while (std::getline(MtlFile, Line))
    {
        if (Line.empty() || Line[0] == '#')
            continue;

        std::istringstream LineStream(Line);
        std::string Token;
        LineStream >> Token;

        // Create new material if token is 'newmtl'
        if (Token == "newmtl")
        {
            LineStream >> Line;
            MaterialIndex++;

            FObjMaterialInfo Material;
            Material.MaterialName = Line;
            OutFStaticMesh.Materials.Add(Material);
        }

        if (Token == "Kd")
        {
            float x, y, z;
            LineStream >> x >> y >> z;
            OutFStaticMesh.Materials[MaterialIndex].Diffuse = FVector(x, y, z);
        }

        if (Token == "Ks")
        {
            float x, y, z;
            LineStream >> x >> y >> z;
            OutFStaticMesh.Materials[MaterialIndex].Specular = FVector(x, y, z);
        }

        if (Token == "Ka")
        {
            float x, y, z;
            LineStream >> x >> y >> z;
            OutFStaticMesh.Materials[MaterialIndex].Ambient = FVector(x, y, z);
        }

        if (Token == "Ke")
        {
            float x, y, z;
            LineStream >> x >> y >> z;
            OutFStaticMesh.Materials[MaterialIndex].Emissive = FVector(x, y, z);
        }

        if (Token == "Ns")
        {
            float x;
            LineStream >> x;
            OutFStaticMesh.Materials[MaterialIndex].SpecularScalar = x;
        }

        if (Token == "sharpness")
        {
            float x;
            LineStream >> x;
            OutFStaticMesh.Materials[MaterialIndex].Sharpness = x;
        }

        if (Token == "Ni")
        {
            float x;
            LineStream >> x;
            OutFStaticMesh.Materials[MaterialIndex].DensityScalar = x;
        }

        if (Token == "d" || Token == "Tr")
        {
            float x;
            LineStream >> x;
            OutFStaticMesh.Materials[MaterialIndex].TransparencyScalar = x;
            OutFStaticMesh.Materials[MaterialIndex].bTransparent = true;
        }

        if (Token == "illum")
        {
            uint32 x;
            LineStream >> x;
            OutFStaticMesh.Materials[MaterialIndex].IlluminanceModel = x;
        }

        if (Token == "map_Kd")
        {
            LineStream >> Line;
            PerTextureData TextureData;
            TextureData.Name = Line;
            FWString TexturePath = OutObjInfo.FilePath + TextureData.Name.ToWideString();
            TextureData.Path = TexturePath;

            if (CreateTextureFromFile(TextureData.Path))
            {
                ETextureFlag::Type TextureFlag = ETextureFlag::Diffuse;
                OutFStaticMesh.Materials[MaterialIndex].TextureFlag |= TextureFlag;
                OutFStaticMesh.Materials[MaterialIndex].TextureData.Add(TextureFlag, TextureData);
            }
        }

        if (Token == "map_Ka")
        {
            LineStream >> Line;
            PerTextureData TextureData;
            TextureData.Name = Line;
            FWString TexturePath = OutObjInfo.FilePath + TextureData.Name.ToWideString();
            TextureData.Path = TexturePath;

            if (CreateTextureFromFile(TextureData.Path))
            {
                ETextureFlag::Type TextureFlag = ETextureFlag::Ambient;
                OutFStaticMesh.Materials[MaterialIndex].TextureFlag |= TextureFlag;
                OutFStaticMesh.Materials[MaterialIndex].TextureData.Add(TextureFlag, TextureData);
            }
        }

        if (Token == "map_Ks")
        {
            LineStream >> Line;
            PerTextureData TextureData;
            TextureData.Name = Line;
            FWString TexturePath = OutObjInfo.FilePath + TextureData.Name.ToWideString();
            TextureData.Path = TexturePath;

            if (CreateTextureFromFile(TextureData.Path))
            {
                ETextureFlag::Type TextureFlag = ETextureFlag::Specular;
                OutFStaticMesh.Materials[MaterialIndex].TextureFlag |= TextureFlag;
                OutFStaticMesh.Materials[MaterialIndex].TextureData.Add(TextureFlag, TextureData);
            }
        }

        if (Token == "map_d")
        {
            LineStream >> Line;
            PerTextureData TextureData;
            TextureData.Name = Line;
            FWString TexturePath = OutObjInfo.FilePath + TextureData.Name.ToWideString();
            TextureData.Path = TexturePath;

            if (CreateTextureFromFile(TextureData.Path))
            {
                ETextureFlag::Type TextureFlag = ETextureFlag::Alpha;
                OutFStaticMesh.Materials[MaterialIndex].TextureFlag |= TextureFlag;
                OutFStaticMesh.Materials[MaterialIndex].TextureData.Add(TextureFlag, TextureData);
            }
        }
        
        if (Token == "map_Ke")
        {
            LineStream >> Line;
            PerTextureData TextureData;
            TextureData.Name = Line;
            FWString TexturePath = OutObjInfo.FilePath + TextureData.Name.ToWideString();
            TextureData.Path = TexturePath;

            if (CreateTextureFromFile(TextureData.Path))
            {
                ETextureFlag::Type TextureFlag = ETextureFlag::Emissive;
                OutFStaticMesh.Materials[MaterialIndex].TextureFlag |= TextureFlag;
                OutFStaticMesh.Materials[MaterialIndex].TextureData.Add(TextureFlag, TextureData);
            }
        }

        if (Token == "map_Ns")
        {
            LineStream >> Line;
            PerTextureData TextureData;
            TextureData.Name = Line;
            FWString TexturePath = OutObjInfo.FilePath + TextureData.Name.ToWideString();
            TextureData.Path = TexturePath;

            if (CreateTextureFromFile(TextureData.Path))
            {
                ETextureFlag::Type TextureFlag = ETextureFlag::Roughness;
                OutFStaticMesh.Materials[MaterialIndex].TextureFlag |= TextureFlag;
                OutFStaticMesh.Materials[MaterialIndex].TextureData.Add(TextureFlag, TextureData);
            }
        }

        if (Token == "map_Bump" or Token == "map_bump")
        {
            FString BumpTextureName;

            // 마지막에 오는 텍스처 파일 이름만 저장
            while (LineStream >> Line)
            {
                if (Line[0] != '-')
                {
                    BumpTextureName = Line;
                }
            }
            
            PerTextureData TextureData;
            TextureData.Name = BumpTextureName;
            FWString TexturePath = OutObjInfo.FilePath + TextureData.Name.ToWideString();
            TextureData.Path = TexturePath;

            if (CreateTextureFromFile(TextureData.Path))
            {
                ETextureFlag::Type TextureFlag = ETextureFlag::Normal;
                OutFStaticMesh.Materials[MaterialIndex].TextureFlag |= TextureFlag;
                OutFStaticMesh.Materials[MaterialIndex].TextureData.Add(TextureFlag, TextureData);
            }
        }
    }

    return true;
}

bool FLoaderOBJ::ConvertToStaticMesh(const FObjInfo& RawData, OBJ::FStaticMeshRenderData& OutStaticMesh)
{
    OutStaticMesh.ObjectName = RawData.ObjectName;
    //OutStaticMesh.PathName = RawData.PathName;
    OutStaticMesh.DisplayName = RawData.DisplayName;

    // 고유 정점을 기반으로 FVertexSimple 배열 생성
    TMap<std::string, uint32> IndexMap; // 중복 체크용

    for (int32 i = 0; i < RawData.VertexIndices.Num(); i++)
    {
        const uint32 VertexIndex = RawData.VertexIndices[i];
        const uint32 UVIndex = RawData.UVIndices[i];
        const uint32 NormalIndex = RawData.NormalIndices[i];

        uint32 MaterialIndex = 0;
        for (int32 j = 0; j < OutStaticMesh.MaterialSubsets.Num(); j++)
        {
            const FMaterialSubset& Subset = OutStaticMesh.MaterialSubsets[j];
            if ( i >= Subset.IndexStart && i < Subset.IndexStart + Subset.IndexCount)
            {
                MaterialIndex = Subset.MaterialIndex;
                break;
            }
        }
        
        // 키 생성 (v/vt/vn 조합)
        std::string Key = std::to_string(VertexIndex) + "/" + std::to_string(UVIndex) + "/" + std::to_string(NormalIndex);
        uint32 FinalIndex;
        if (IndexMap.Contains(Key) && OutStaticMesh.Materials.Num() > MaterialIndex && !(OutStaticMesh.Materials[MaterialIndex].TextureFlag & ETextureFlag::Normal) )
        {
            FinalIndex = IndexMap[Key];
        }
        else
        {
            FStaticMeshVertex StaticMeshVertex = {};
            StaticMeshVertex.MaterialIndex = MaterialIndex;
            StaticMeshVertex.X = RawData.Vertices[VertexIndex].X;
            StaticMeshVertex.Y = RawData.Vertices[VertexIndex].Y;
            StaticMeshVertex.Z = RawData.Vertices[VertexIndex].Z;

            StaticMeshVertex.R = 0.0f; StaticMeshVertex.G = 0.0f; StaticMeshVertex.B = 0.0f; StaticMeshVertex.A = 1.0f; // 기본 색상

            if (UVIndex != UINT32_MAX && UVIndex < RawData.UVs.Num())
            {
                StaticMeshVertex.U = RawData.UVs[UVIndex].X;
                StaticMeshVertex.V = RawData.UVs[UVIndex].Y;
            }

            if (NormalIndex != UINT32_MAX && NormalIndex < RawData.Normals.Num())
            {
                StaticMeshVertex.NormalX = RawData.Normals[NormalIndex].X;
                StaticMeshVertex.NormalY = RawData.Normals[NormalIndex].Y;
                StaticMeshVertex.NormalZ = RawData.Normals[NormalIndex].Z;
            }

            if (i % 3 == 2) // 삼각형이 구성되면 Tangent 계산
            {
                const uint32 IndexNum = OutStaticMesh.Indices.Num();

                FStaticMeshVertex& Vertex0 = OutStaticMesh.Vertices[OutStaticMesh.Indices[IndexNum - 2]];
                FStaticMeshVertex& Vertex1 = OutStaticMesh.Vertices[OutStaticMesh.Indices[IndexNum - 1]];
                FStaticMeshVertex& Vertex2 = StaticMeshVertex;

                CalculateTangent(Vertex0, Vertex1, Vertex2);
                CalculateTangent(Vertex1, Vertex2, Vertex0);
                CalculateTangent(Vertex2, Vertex0, Vertex1);
            }

            FinalIndex = OutStaticMesh.Vertices.Num();
            OutStaticMesh.Vertices.Add(StaticMeshVertex);
            IndexMap[Key] = FinalIndex;
        }

        OutStaticMesh.Indices.Add(FinalIndex);
    }

    // Calculate StaticMesh BoundingBox
    ComputeBoundingBox(OutStaticMesh.Vertices, OutStaticMesh.BoundingBoxMin, OutStaticMesh.BoundingBoxMax);

    return true;
}

bool FLoaderOBJ::CreateTextureFromFile(const FWString& Filename)
{
    if (FEngineLoop::ResourceManager.GetTexture(Filename))
    {
        return true;
    }

    HRESULT hr = FEngineLoop::ResourceManager.LoadTextureFromFile(FEngineLoop::GraphicDevice.Device, nullptr, Filename.c_str());

    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

void FLoaderOBJ::ComputeBoundingBox(const TArray<FStaticMeshVertex>& InVertices, FVector& OutMinVector, FVector& OutMaxVector)
{
    FVector MinVector = { FLT_MAX, FLT_MAX, FLT_MAX };
    FVector MaxVector = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (int32 i = 0; i < InVertices.Num(); i++)
    {
        MinVector.X = std::min(MinVector.X, InVertices[i].X);
        MinVector.Y = std::min(MinVector.Y, InVertices[i].Y);
        MinVector.Z = std::min(MinVector.Z, InVertices[i].Z);

        MaxVector.X = std::max(MaxVector.X, InVertices[i].X);
        MaxVector.Y = std::max(MaxVector.Y, InVertices[i].Y);
        MaxVector.Z = std::max(MaxVector.Z, InVertices[i].Z);
    }

    OutMinVector = MinVector;
    OutMaxVector = MaxVector;
}

void FLoaderOBJ::CalculateTangent(FStaticMeshVertex& PivotVertex, const FStaticMeshVertex& Vertex1, const FStaticMeshVertex& Vertex2)
{
    const float s1 = Vertex1.U - PivotVertex.U;
    const float t1 = Vertex1.V - PivotVertex.V;
    const float s2 = Vertex2.U - PivotVertex.U;
    const float t2 = Vertex2.V - PivotVertex.V;

    float denominator = (s1 * t2 - s2 * t1);
    if (FMath::Abs(denominator) < SMALL_NUMBER)
    {
        // 유효하지 않은 경우, 기본값을 사용하거나 생략
        PivotVertex.TangentX = 1.f;
        PivotVertex.TangentY = 0.f;
        PivotVertex.TangentZ = 0.f;
        return;
    }

    const float f = 1.f / denominator;
    
    const float E1x = Vertex1.X - PivotVertex.X;
    const float E1y = Vertex1.Y - PivotVertex.Y;
    const float E1z = Vertex1.Z - PivotVertex.Z;
    const float E2x = Vertex2.X - PivotVertex.X;
    const float E2y = Vertex2.Y - PivotVertex.Y;
    const float E2z = Vertex2.Z - PivotVertex.Z;

    const float Tx = f * (t2 * E1x - t1 * E2x);
    const float Ty = f * (t2 * E1y - t1 * E2y);
    const float Tz = f * (t2 * E1z - t1 * E2z);

    // Todo: 이거 필수로 고쳐야함 !! (1,1,1) 값만 들어가는 중...
    FVector Tangent = FVector(Tx, Ty, Tz).Normalize();

    PivotVertex.TangentX = Tangent.X;
    PivotVertex.TangentY = Tangent.Y;
    PivotVertex.TangentZ = Tangent.Z;
}

OBJ::FStaticMeshRenderData* FManagerOBJ::LoadObjStaticMeshAsset(const FString& PathFileName)
{
    OBJ::FStaticMeshRenderData* NewStaticMesh = new OBJ::FStaticMeshRenderData();

    if ( const auto It = ObjStaticMeshMap.Find(PathFileName))
    {
        return *It;
    }

    FWString BinaryPath = (PathFileName + ".bin").ToWideString();
    if (std::ifstream(BinaryPath).good())
    {
        if (LoadStaticMeshFromBinary(BinaryPath, *NewStaticMesh))
        {
            ObjStaticMeshMap.Add(PathFileName, NewStaticMesh);
            return NewStaticMesh;
        }
    }

    // Parse OBJ
    FObjInfo NewObjInfo;
    bool Result = FLoaderOBJ::ParseOBJ(PathFileName, NewObjInfo);

    if (!Result)
    {
        delete NewStaticMesh;
        return nullptr;
    }

    // Material
    if (NewObjInfo.MaterialSubsets.Num() > 0)
    {
        Result = FLoaderOBJ::ParseMaterial(NewObjInfo, *NewStaticMesh);

        if (!Result)
        {
            delete NewStaticMesh;
            return nullptr;
        }

        CombineMaterialIndex(*NewStaticMesh);

        for (int materialIndex = 0; materialIndex < NewStaticMesh->Materials.Num(); materialIndex++) {
            CreateMaterial(NewStaticMesh->Materials[materialIndex]);
        }
    }

    // Convert FStaticMeshRenderData
    Result = FLoaderOBJ::ConvertToStaticMesh(NewObjInfo, *NewStaticMesh);
    if (!Result)
    {
        delete NewStaticMesh;
        return nullptr;
    }

    // SaveStaticMeshToBinary(BinaryPath, *NewStaticMesh); // TODO: refactoring 끝나면 활성화하기
    ObjStaticMeshMap.Add(PathFileName, NewStaticMesh);
    return NewStaticMesh;
}

void FManagerOBJ::CombineMaterialIndex(OBJ::FStaticMeshRenderData& OutFStaticMesh)
{
    for (int32 i = 0; i < OutFStaticMesh.MaterialSubsets.Num(); i++)
    {
        FString MatName = OutFStaticMesh.MaterialSubsets[i].MaterialName;
        for (int32 j = 0; j < OutFStaticMesh.Materials.Num(); j++)
        {
            if (OutFStaticMesh.Materials[j].MaterialName == MatName)
            {
                OutFStaticMesh.MaterialSubsets[i].MaterialIndex = j;
                break;
            }
        }
    }
}

bool FManagerOBJ::SaveStaticMeshToBinary(const FWString& FilePath, const OBJ::FStaticMeshRenderData& StaticMesh)
{
    std::ofstream File(FilePath, std::ios::binary);
    if (!File.is_open())
    {
        assert("CAN'T SAVE STATIC MESH BINARY FILE");
        return false;
    }

    // Object Name
    Serializer::WriteFWString(File, StaticMesh.ObjectName);

    // Display Name
    Serializer::WriteFString(File, StaticMesh.DisplayName);

    // Vertices
    uint32 VertexCount = StaticMesh.Vertices.Num();
    File.write(reinterpret_cast<const char*>(&VertexCount), sizeof(VertexCount));
    File.write(reinterpret_cast<const char*>(StaticMesh.Vertices.GetData()), VertexCount * sizeof(FStaticMeshVertex));

    // Indices
    uint32 IndexCount = StaticMesh.Indices.Num();
    File.write(reinterpret_cast<const char*>(&IndexCount), sizeof(IndexCount));
    File.write(reinterpret_cast<const char*>(StaticMesh.Indices.GetData()), IndexCount * sizeof(UINT));

    // Materials
    uint32 MaterialCount = StaticMesh.Materials.Num();
    File.write(reinterpret_cast<const char*>(&MaterialCount), sizeof(MaterialCount));
    for (const FObjMaterialInfo& Material : StaticMesh.Materials)
    {
        Serializer::WriteFString(File, Material.MaterialName);
        File.write(reinterpret_cast<const char*>(&Material.bTransparent), sizeof(Material.bTransparent));
        File.write(reinterpret_cast<const char*>(&Material.Diffuse), sizeof(Material.Diffuse));
        File.write(reinterpret_cast<const char*>(&Material.Specular), sizeof(Material.Specular));
        File.write(reinterpret_cast<const char*>(&Material.Ambient), sizeof(Material.Ambient));
        File.write(reinterpret_cast<const char*>(&Material.Emissive), sizeof(Material.Emissive));
        File.write(reinterpret_cast<const char*>(&Material.SpecularScalar), sizeof(Material.SpecularScalar));
        File.write(reinterpret_cast<const char*>(&Material.DensityScalar), sizeof(Material.DensityScalar));
        File.write(reinterpret_cast<const char*>(&Material.Sharpness), sizeof(Material.Sharpness));
        File.write(reinterpret_cast<const char*>(&Material.TransparencyScalar), sizeof(Material.TransparencyScalar));
        File.write(reinterpret_cast<const char*>(&Material.IlluminanceModel), sizeof(Material.IlluminanceModel));

        for (auto& [TextureFlag, TextureData] : Material.TextureData)
        {
            Serializer::WriteFString(File, TextureData.Name);
            Serializer::WriteFWString(File, TextureData.Path);
        }
    }

    // Material Subsets
    uint32 SubsetCount = StaticMesh.MaterialSubsets.Num();
    File.write(reinterpret_cast<const char*>(&SubsetCount), sizeof(SubsetCount));
    for (const FMaterialSubset& Subset : StaticMesh.MaterialSubsets)
    {
        Serializer::WriteFString(File, Subset.MaterialName);
        File.write(reinterpret_cast<const char*>(&Subset.IndexStart), sizeof(Subset.IndexStart));
        File.write(reinterpret_cast<const char*>(&Subset.IndexCount), sizeof(Subset.IndexCount));
        File.write(reinterpret_cast<const char*>(&Subset.MaterialIndex), sizeof(Subset.MaterialIndex));
    }

    // Bounding Box
    File.write(reinterpret_cast<const char*>(&StaticMesh.BoundingBoxMin), sizeof(FVector));
    File.write(reinterpret_cast<const char*>(&StaticMesh.BoundingBoxMax), sizeof(FVector));

    File.close();
    return true;
}

bool FManagerOBJ::LoadStaticMeshFromBinary(const FWString& FilePath, OBJ::FStaticMeshRenderData& OutStaticMesh)
{
    std::ifstream File(FilePath, std::ios::binary);
    if (!File.is_open())
    {
        assert("CAN'T OPEN STATIC MESH BINARY FILE");
        return false;
    }

    TArray<FWString> Textures;

    // Object Name
    Serializer::ReadFWString(File, OutStaticMesh.ObjectName);

    //// Path Name
    //Serializer::ReadFWString(File, OutStaticMesh.PathName);

    // Display Name
    Serializer::ReadFString(File, OutStaticMesh.DisplayName);

    // Vertices
    uint32 VertexCount = 0;
    File.read(reinterpret_cast<char*>(&VertexCount), sizeof(VertexCount));
    OutStaticMesh.Vertices.SetNum(VertexCount);
    File.read(reinterpret_cast<char*>(OutStaticMesh.Vertices.GetData()), VertexCount * sizeof(FStaticMeshVertex));

    // Indices
    uint32 IndexCount = 0;
    File.read(reinterpret_cast<char*>(&IndexCount), sizeof(IndexCount));
    OutStaticMesh.Indices.SetNum(IndexCount);
    File.read(reinterpret_cast<char*>(OutStaticMesh.Indices.GetData()), IndexCount * sizeof(UINT));

    // Material
    uint32 MaterialCount = 0;
    File.read(reinterpret_cast<char*>(&MaterialCount), sizeof(MaterialCount));
    OutStaticMesh.Materials.SetNum(MaterialCount);
    for (FObjMaterialInfo& Material : OutStaticMesh.Materials)
    {
        Serializer::ReadFString(File, Material.MaterialName);
        File.read(reinterpret_cast<char*>(&Material.bTransparent), sizeof(Material.bTransparent));
        File.read(reinterpret_cast<char*>(&Material.Diffuse), sizeof(Material.Diffuse));
        File.read(reinterpret_cast<char*>(&Material.Specular), sizeof(Material.Specular));
        File.read(reinterpret_cast<char*>(&Material.Ambient), sizeof(Material.Ambient));
        File.read(reinterpret_cast<char*>(&Material.Emissive), sizeof(Material.Emissive));
        File.read(reinterpret_cast<char*>(&Material.SpecularScalar), sizeof(Material.SpecularScalar));
        File.read(reinterpret_cast<char*>(&Material.DensityScalar), sizeof(Material.DensityScalar));
        File.read(reinterpret_cast<char*>(&Material.Sharpness), sizeof(Material.Sharpness));
        File.read(reinterpret_cast<char*>(&Material.TransparencyScalar), sizeof(Material.TransparencyScalar));
        File.read(reinterpret_cast<char*>(&Material.IlluminanceModel), sizeof(Material.IlluminanceModel));

        for (auto& [TextureFlag, TextureData] : Material.TextureData)
        {
            Serializer::ReadFString(File, TextureData.Name);
            Serializer::ReadFWString(File, TextureData.Path);

            if (!TextureData.Path.empty())
            {
                Textures.AddUnique(TextureData.Path);
            }
        }
    }

    // Material Subset
    uint32 SubsetCount = 0;
    File.read(reinterpret_cast<char*>(&SubsetCount), sizeof(SubsetCount));
    OutStaticMesh.MaterialSubsets.SetNum(SubsetCount);
    for (FMaterialSubset& Subset : OutStaticMesh.MaterialSubsets)
    {
        Serializer::ReadFString(File, Subset.MaterialName);
        File.read(reinterpret_cast<char*>(&Subset.IndexStart), sizeof(Subset.IndexStart));
        File.read(reinterpret_cast<char*>(&Subset.IndexCount), sizeof(Subset.IndexCount));
        File.read(reinterpret_cast<char*>(&Subset.MaterialIndex), sizeof(Subset.MaterialIndex));
    }

    // Bounding Box
    File.read(reinterpret_cast<char*>(&OutStaticMesh.BoundingBoxMin), sizeof(FVector));
    File.read(reinterpret_cast<char*>(&OutStaticMesh.BoundingBoxMax), sizeof(FVector));

    File.close();

    // Texture Load
    if (Textures.Num() > 0)
    {
        for (const FWString& Texture : Textures)
        {
            if (FEngineLoop::ResourceManager.GetTexture(Texture) == nullptr)
            {
                FEngineLoop::ResourceManager.LoadTextureFromFile(FEngineLoop::GraphicDevice.Device, nullptr, Texture.c_str());
            }
        }
    }

    return true;
}

UMaterial* FManagerOBJ::CreateMaterial(FObjMaterialInfo materialInfo)
{
    if (materialMap[materialInfo.MaterialName] != nullptr)
        return materialMap[materialInfo.MaterialName];

    UMaterial* newMaterial = FObjectFactory::ConstructObject<UMaterial>(nullptr); // Material은 Outer가 없이 따로 관리되는 객체이므로 Outer가 없음으로 설정. 추후 Garbage Collection이 추가되면 AssetManager를 생성해서 관리.
    newMaterial->SetMaterialInfo(materialInfo);
    materialMap.Add(materialInfo.MaterialName, newMaterial);
    return newMaterial;
}

UMaterial* FManagerOBJ::GetMaterial(FString name)
{
    return materialMap[name];
}

UStaticMesh* FManagerOBJ::CreateStaticMesh(const FString& filePath)
{
    OBJ::FStaticMeshRenderData* StaticMeshRenderData = FManagerOBJ::LoadObjStaticMeshAsset(filePath);

    if (StaticMeshRenderData == nullptr) return nullptr;

    UStaticMesh* StaticMesh = GetStaticMesh(StaticMeshRenderData->ObjectName);
    if (StaticMesh != nullptr)
    {
        return StaticMesh;
    }

    StaticMesh = FObjectFactory::ConstructObject<UStaticMesh>(nullptr); // TODO: 추후 AssetManager를 생성해서 관리.
    StaticMesh->SetData(StaticMeshRenderData);

    StaticMeshMap.Add(StaticMeshRenderData->ObjectName, StaticMesh); // TODO: 장기적으로 보면 파일 이름 대신 경로를 Key로 사용하는게 좋음.
    return StaticMesh;
}

UStaticMesh* FManagerOBJ::GetStaticMesh(FWString name)
{
    return StaticMeshMap[name];
}
