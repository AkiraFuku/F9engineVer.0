#include "Model.h"
#include "TextureManager.h"
#include "DXCommon.h"
#include "MathFunction.h"
#include <cassert>
#include <fstream> 
#include <sstream>
#include <Windows.h>
#include <numbers>
#include <imgui.h>


void Model::Initialize(const std::string& directryPath, const std::string& filename)
{

    name_=filename;
    modelData_ = LoadModelFile(directryPath, filename);
    if (modelData_.material.textureFilePath.empty()) {
        modelData_.material.textureFilePath = "resources/uvChecker.png"; // 確実に存在する画像を指定
        TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);



    }  //頂点リソースの作成
    CreateVertexBuffer();
    //マテリアルリソースの作成
    CreateMaterialResource();
    //テクスチャの読み込み
    TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
    //テクスチャインデックスの取得
    modelData_.material.textureIndex =
        TextureManager::GetInstance()->GetTextureIndexByFilePath(
            modelData_.material.textureFilePath);

}

void Model::Update()
{
#ifdef USE_IMGUI
    std::string title=name_;
    ImGui::Begin(title.c_str());
            int* pEnableLighting = reinterpret_cast<int*>(&materialData_->enableLighting);
            ImGui::Checkbox("Enable Lighting", (bool*)pEnableLighting);
            if (materialData_->enableLighting) {
                // 拡散反射 (Diffuse) の設定
                ImGui::Text("Diffuse (Base)");
                const char* diffuseItems[] = { "Lambert", "Half-Lambert" };
                ImGui::Combo("Diffuse Type", &materialData_->diffuseType, diffuseItems, IM_ARRAYSIZE(diffuseItems));

                // 鏡面反射 (Specular) の設定
                ImGui::Text("Specular (Shininess)");
                const char* specularItems[] = { "None", "Phong", "Blinn-Phong" };
                ImGui::Combo("Specular Type", &materialData_->specularType, specularItems, IM_ARRAYSIZE(specularItems));

                // 光沢度
                ImGui::DragFloat("Shininess", &materialData_->shininess, 0.1f, 1.0f, 256.0f);
            }
          

            ImGui::End();


#endif // USE_IMGUI


}
void Model::Draw() {
    //VBVの設定
    DXCommon::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
    //マテリアルリソースの設定
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_.Get()->GetGPUVirtualAddress());
    //SRVのディスクリプタテーブルの設定
    DXCommon::GetInstance()->
        GetCommandList()->
        SetGraphicsRootDescriptorTable(2,
            TextureManager::GetInstance()->GetSrvHandleGPU(modelData_.material.textureIndex));
    //描画コマンド
    DXCommon::GetInstance()->GetCommandList()->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);

}

void Model::CreateVertexBuffer() {
    //頂点リソースの作成
    vertexResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());
    //頂点バッファビューの設定
    vertexBufferView_.BufferLocation =
        vertexResource_.Get()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
    vertexResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

    //頂点データの転送
    memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

void Model::CreateMaterialResource() {
    //マテリアルリソースの作成
    materialResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(Material));
    materialResource_->
        Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

    materialData_->color = Vector4{ 1.0f,1.0f,1.0f,1.0f };
    materialData_->enableLighting = true;
    materialData_->uvTransform = Makeidetity4x4();
    materialData_->shininess=50.0f;
    materialData_->specularType=BlinnPhong;
    materialData_->diffuseType=HarfLambert;

}
Model::MaterialData  Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    //1. 変数の宣言
    MaterialData materialData{}; // 修正: 初期化
    std::string line;
    std::ifstream file(directoryPath + "/" + filename);//ファイルパスを結合して開く
    //2. ファイルを開く
    assert(file.is_open());//ファイルが開けたか確認
    //3. ファイルからデータを読み込みマテリアルデータを作成
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;//行の先頭を識別子として取得

        if (identifier == "map_Kd") {
            std::string textureFileName;
            s >> textureFileName;//テクスチャファイル名を読み込み
            //テクスチャのパスを設定
            materialData.textureFilePath = directoryPath + "/" + textureFileName;
        }
    }

    //4. マテリアルデータを返す
    return materialData;
}

Model::ModelData Model::LoadModelFile(const std::string& directoryPath, const std::string& filename)
{
       //1. 変数の宣言
    ModelData modelData;
    std::string filePath = directoryPath + "/" + filename;

    ////2. ファイルを開く
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filePath.c_str(),
        aiProcess_FlipWindingOrder |              // 三角形化されていないポリゴンを三角形にする
        aiProcess_FlipUVs        |        // 法線がない場合、自動計算する
aiProcess_PreTransformVertices    
    );
    assert(scene->HasMeshes());
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        assert(mesh->HasNormals());
        assert(mesh->HasTextureCoords(0));
        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces;++faceIndex)
        {
            aiFace& face = mesh->mFaces[faceIndex];
            assert(face.mNumIndices == 3);
            for (uint32_t element = 0; element < face.mNumIndices; ++element)
            {
                uint32_t vertexIndex = face.mIndices[element];
                aiVector3D& position = mesh->mVertices[vertexIndex];
                aiVector3D& normal = mesh->mNormals[vertexIndex];
                aiVector3D& texcord = mesh->mTextureCoords[0][vertexIndex];
                VertexData vertex;
                vertex.position = { position.x,position.y,position.z,1.0f };
                vertex.normal = { normal.x,normal.y,normal.z };
                vertex.texcord = { texcord.x,texcord.y };
                vertex.position.x *= -1.0f;
                vertex.normal.x *= -1.0f;
                modelData.vertices.push_back(vertex);
            }
        }
    }
    for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
    {
        aiMaterial* material = scene->mMaterials[materialIndex];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
        {
            aiString textureFilePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
            modelData.material.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
        }
    }
    modelData.rootNode = ReadNode(scene->mRootNode);
   // 4. モデルデータを返す
    return modelData;

}

Model* Model::CreateSphere(uint32_t subdivision)
{
    Model* model = new Model();

    model->name_="Sphere";
    // 1. メモリ確保（頂点リソース作成など既存のInitializeの一部が必要だが、
    // ここではvertex生成に集中し、後でリソース生成関数を呼ぶ流れにします）
    // ※TextureManagerへの依存があるため、適当な白画像などをデフォルトにする必要があります
     
    model->modelData_.material.textureFilePath = "resources/monsterBall.png"; // 確実に存在する画像を指定
   // TextureManagerを使ってテクスチャを読み込む
    TextureManager::GetInstance()->LoadTexture( model->modelData_.material.textureFilePath);

    // 読み込んだテクスチャのSRVインデックスを取得して設定する
    model->modelData_.material.textureIndex = 
        TextureManager::GetInstance()->GetTextureIndexByFilePath( model->modelData_.material.textureFilePath);

    // 分割数に応じた角度の刻み幅
    const float kLonEvery = 2.0f * std::numbers::pi_v<float> / float(subdivision);
    const float kLatEvery = std::numbers::pi_v<float> / float(subdivision);

    // 緯度方向のループ
    for (uint32_t latIndex = 0; latIndex < subdivision; ++latIndex) {
        float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex; // 現在の緯度 theta

        // 経度方向のループ
        for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
            float lon = lonIndex * kLonEvery; // 現在の経度 phi

            // 1枚の四角形を作るための4点の座標を計算
            // a -- b
            // |    |
            // c -- d
            // のような位置関係の4点を求めます

            // 便利関数（ラムダ式）: 緯度・経度から頂点データを作る
            auto makeVertex = [&](float u, float v, float latitude, float longitude) {
                VertexData vertex;
                // 座標計算 (半径1.0と仮定)
                vertex.position.x = std::cos(latitude) * std::cos(longitude);
                vertex.position.y = std::sin(latitude);
                vertex.position.z = std::cos(latitude) * std::sin(longitude);
                vertex.position.w = 1.0f;

                // 法線（球体なので原点から座標へのベクトルと同じ向き）
                vertex.normal.x = vertex.position.x;
                vertex.normal.y = vertex.position.y;
                vertex.normal.z = vertex.position.z;

                // UV座標
                vertex.texcord = { u, 1.0f - v }; // DXはVが逆の場合があるため適宜調整
                return vertex;
                };

            // 4点のUVと角度を算出
            float u0 = float(lonIndex) / float(subdivision);
            float v0 = float(latIndex) / float(subdivision);
            float u1 = float(lonIndex + 1) / float(subdivision);
            float v1 = float(latIndex + 1) / float(subdivision);

            // 点A (左下)
            VertexData a = makeVertex(u0, v0, lat, lon);
            // 点B (左上) ※緯度はlat + kLatEvery
            VertexData b = makeVertex(u0, v1, lat + kLatEvery, lon);
            // 点C (右下) ※経度はlon + kLonEvery
            VertexData c = makeVertex(u1, v0, lat, lon + kLonEvery);
            // 点D (右上)
            VertexData d = makeVertex(u1, v1, lat + kLatEvery, lon + kLonEvery);

            // 頂点を追加 (Triangle List: 2つの三角形で四角形を作る)
            // 三角形1 (A, B, C)
            model->modelData_.vertices.push_back(a);
            model->modelData_.vertices.push_back(b);
            model->modelData_.vertices.push_back(c);

            // 三角形2 (C, B, D)
            model->modelData_.vertices.push_back(c);
            model->modelData_.vertices.push_back(b);
            model->modelData_.vertices.push_back(d);
        }
    }

    // 既存のメソッドを利用してGPUバッファを作成
    // ※Initialize関数の中身を分解するか、この関数内で CreateVertexBuffer() 等を呼べるようにアクセス権を調整してください
    model->CreateVertexBuffer();
    model->CreateMaterialResource();

    return model;
}

 Model::Node Model::ReadNode(aiNode* node)
{
   Node result;
  
    aiMatrix4x4 aiLocalMatrix = node->mTransformation;
    aiLocalMatrix.Transpose();

 
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            // Assimpの行列は [row][col] でアクセス可能
            // 自作Matrix構造体の定義に合わせて代入 (例: result.localMatrix.m[i][j])
            result.localMatrix.m[i][j] = aiLocalMatrix[i][j];
        }
    }
    result.name = node->mName.C_Str();
    result.children.resize(node->mNumChildren);
    for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
    {
        result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
    }
    return result;
}

