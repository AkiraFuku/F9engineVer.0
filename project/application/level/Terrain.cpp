#define NOMINMAX
#include "Terrain.h"
#include "PSOManager.h"
#include "TextureManager.h"
#include "DXCommon.h"
#include "MathFunction.h"
#include <sstream>
#include <algorithm>

void Terrain::Initialize(const LevelObjectData& data, Camera* camera)
{
    name_     = data.name;
    camera_   = camera;

    transform_.translate = data.transform.translation;
    transform_.rotate    = data.transform.rotation;
    transform_.scale     = data.transform.scale;

    SetupPSO();
    BuildMesh(data);
    Update();
}

void Terrain::SetupPSO()
{
    // 既に "Terrain" PSO が登録済みなら再登録しない
    static bool s_psoRegistered = false;
    if (s_psoRegistered) {
        return;
    }
    s_psoRegistered = true;

    PsoConfig PSO{};
    PsoConfig::ShaderPath vsPath{ ShaderType::VS, L"resources/shaders/Terrain/Terrain.vs.hlsl", "main", L"vs_6_0" };
    PsoConfig::ShaderPath psPath{ ShaderType::PS, L"resources/shaders/Terrain/Terrain.ps.hlsl", "main", L"ps_6_0" };
    PSO.shaderPaths.push_back(vsPath);
    PSO.shaderPaths.push_back(psPath);

    PSO.rootSignatureGenerator = []() {
        return RootSignatureBuilder()
            // [Param 0] Material (CBV b0, Pixel)
            .AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)
            // [Param 1] Transform (CBV b1, Vertex)
            .AddCBV(1, D3D12_SHADER_VISIBILITY_VERTEX)
            // [Param 2] Grass Texture (DescriptorTable t0, Pixel)
            .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            // [Param 3] Road Texture (DescriptorTable t1, Pixel)
            .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL)
            // スタティックサンプラー (s0)
            .AddStaticSampler(PSOManager::GetInstance()->StaticSamplers())
            .Build(DXCommon::GetInstance()->GetDevice().Get());
    };

    PSO.inputLayoutGenerator = []() {
        InputLayout inputLayout = {};
        inputLayout.inputElement = {
            { "POSITION",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",  1, DXGI_FORMAT_R32_FLOAT,          0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        inputLayout.inputLayout.pInputElementDescs = inputLayout.inputElement.data();
        inputLayout.inputLayout.NumElements = static_cast<UINT>(inputLayout.inputElement.size());
        return inputLayout;
    };

    PSO.cullMode = D3D12_CULL_MODE_NONE; // 両面描画
    PSO.depthEnable = true;
    PSO.depthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    PSO.depth.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    PSOManager::GetInstance()->RegisterPsoGenerator("Terrain", PSO);
}

void Terrain::BuildMesh(const LevelObjectData& data)
{
    auto getFloat = [&](const std::string& key, float def) -> float {
        auto it = data.properties.find(key);
        if (it != data.properties.end()) {
            try { return std::stof(it->second); } catch (...) {}
        }
        return def;
    };
    auto getInt = [&](const std::string& key, int def) -> int {
        auto it = data.properties.find(key);
        if (it != data.properties.end()) {
            try { return std::stoi(it->second); } catch (...) {}
        }
        return def;
    };

    sizeX_  = getFloat("size_x",      160.0f);
    sizeY_  = getFloat("size_y",      240.0f);
    divX_   = getInt  ("divisions_x", 64);
    divY_   = getInt  ("divisions_y", 80);
    uvTile_ = getFloat("uv_tile",     10.0f);

    // テクスチャ設定
    auto itGrass = data.properties.find("grass_texture");
    grassTexturePath_ = (itGrass != data.properties.end()) ? itGrass->second : (!data.texturePath.empty() ? data.texturePath : "resources/Stagemap/863603.png");

    auto itRoad = data.properties.find("road_texture");
    roadTexturePath_ = (itRoad != data.properties.end()) ? itRoad->second : "resources/grass.png";

    TextureManager::GetInstance()->LoadTexture(grassTexturePath_);
    TextureManager::GetInstance()->LoadTexture(roadTexturePath_);
    grassTexIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(grassTexturePath_);
    roadTexIndex_  = TextureManager::GetInstance()->GetTextureIndexByFilePath(roadTexturePath_);

    // ─── セル属性（cell_types）パース ────────────────────────────────
    int totalCells = divX_ * divY_;
    cellTypes_.assign(totalCells, CellType::kGround);

    auto itCells = data.properties.find("cell_types");
    if (itCells != data.properties.end() && !itCells->second.empty()) {
        std::stringstream ss(itCells->second);
        std::string token;
        int idx = 0;
        while (std::getline(ss, token, ',') && idx < totalCells) {
            try {
                int val = std::stoi(token);
                cellTypes_[idx] = static_cast<CellType>(val);
            } catch (...) {}
            idx++;
        }
    }

    // ─── 頂点生成 ─────────────────────────────────────────────────────
    const int colCount = divX_ + 1;
    const int rowCount = divY_ + 1;
    const float halfX = sizeX_ * 0.5f;
    const float halfZ = sizeY_ * 0.5f;

    vertices_.clear();
    vertices_.reserve(static_cast<size_t>(colCount) * rowCount);

    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            float tx = static_cast<float>(col) / static_cast<float>(divX_);
            float tz = static_cast<float>(row) / static_cast<float>(divY_);

            VertexData v;
            v.position = { -halfX + tx * sizeX_, 0.0f, -halfZ + tz * sizeY_, 1.0f };
            v.texCoord = { tx, tz };
            v.normal   = { 0.0f, 1.0f, 0.0f };

            // この頂点に隣接する4セルのうち、1つでも道なら頂点道属性を1.0にする
            float roadWeight = 0.0f;
            int roadCount = 0;
            int totalNeighbors = 0;

            for (int dy = -1; dy <= 0; ++dy) {
                for (int dx = -1; dx <= 0; ++dx) {
                    int c = col + dx;
                    int r = row + dy;
                    if (c >= 0 && c < divX_ && r >= 0 && r < divY_) {
                        int cIdx = r * divX_ + c;
                        CellType ct = cellTypes_[cIdx];
                        if (ct == CellType::kRoad) {
                            roadCount++;
                        }
                        totalNeighbors++;
                    }
                }
            }
            if (totalNeighbors > 0 && roadCount > 0) {
                roadWeight = static_cast<float>(roadCount) / static_cast<float>(totalNeighbors);
                // 道の存在感を強調するため、半分以上が道なら1.0に
                if (roadCount >= 1) {
                    roadWeight = (std::max)(roadWeight, 0.7f);
                }
            }
            v.roadAttr = roadWeight;

            vertices_.push_back(v);
        }
    }

    // ─── インデックス生成 ＆ 当たり判定用三角形構築 ─────────────────
    indices_.clear();
    indices_.reserve(static_cast<size_t>(totalCells) * 6);
    triangles_.clear();

    Matrix4x4 worldMat = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    auto addTriangle = [&](uint32_t i0, uint32_t i1, uint32_t i2) {
        indices_.push_back(i0);
        indices_.push_back(i1);
        indices_.push_back(i2);

        // ワールド座標系に変換した当たり判定三角形を追加
        Triangle tri;
        Vector3 p0 = { vertices_[i0].position.x, vertices_[i0].position.y, vertices_[i0].position.z };
        Vector3 p1 = { vertices_[i1].position.x, vertices_[i1].position.y, vertices_[i1].position.z };
        Vector3 p2 = { vertices_[i2].position.x, vertices_[i2].position.y, vertices_[i2].position.z };
        tri.vertices[0] = vector3Transform(p0, worldMat);
        tri.vertices[1] = vector3Transform(p1, worldMat);
        tri.vertices[2] = vector3Transform(p2, worldMat);
        triangles_.push_back(tri);
    };

    for (int row = 0; row < divY_; ++row) {
        for (int col = 0; col < divX_; ++col) {
            int cellIdx = row * divX_ + col;
            CellType ctype = (cellIdx < static_cast<int>(cellTypes_.size())) ? cellTypes_[cellIdx] : CellType::kGround;

            // 穴（HOLE）なら完全にスキップ
            if (ctype == CellType::kHole) {
                continue;
            }

            uint32_t lb = static_cast<uint32_t>(row * colCount + col);
            uint32_t rb = static_cast<uint32_t>(row * colCount + col + 1);
            uint32_t lt = static_cast<uint32_t>((row + 1) * colCount + col);
            uint32_t rt = static_cast<uint32_t>((row + 1) * colCount + col + 1);

            // tri0: 左手前 → 左奥 → 右手前 (lb, lt, rb)
            // tri1: 右手前 → 左奥 → 右奥   (rb, lt, rt)
            if (ctype == CellType::kEdgeTri0) {
                // 穴の縁: 三角形0のみ生成（片方の三角ポリゴンで縁を埋める）
                addTriangle(lb, lt, rb);
            } else if (ctype == CellType::kEdgeTri1) {
                // 穴の縁: 三角形1のみ生成
                addTriangle(rb, lt, rt);
            } else {
                // 通常の地面または道: 両方の三角形を生成
                addTriangle(lb, lt, rb);
                addTriangle(rb, lt, rt);
            }
        }
    }

    // ─── DirectX12 バッファリソース生成 ───────────────────────────────
    auto device = DXCommon::GetInstance()->GetDevice().Get();

    // 頂点バッファ
    UINT vbSize = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
    vertexResource_ = DXCommon::GetInstance()->CreateBufferResource(vbSize);
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes    = vbSize;
    vertexBufferView_.StrideInBytes  = sizeof(VertexData);

    void* vbData = nullptr;
    vertexResource_->Map(0, nullptr, &vbData);
    memcpy(vbData, vertices_.data(), vbSize);
    vertexResource_->Unmap(0, nullptr);

    // インデックスバッファ
    if (!indices_.empty()) {
        UINT ibSize = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
        indexResource_ = DXCommon::GetInstance()->CreateBufferResource(ibSize);
        indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
        indexBufferView_.SizeInBytes    = ibSize;
        indexBufferView_.Format         = DXGI_FORMAT_R32_UINT;

        void* ibData = nullptr;
        indexResource_->Map(0, nullptr, &ibData);
        memcpy(ibData, indices_.data(), ibSize);
        indexResource_->Unmap(0, nullptr);
    }

    // マテリアルバッファ
    materialResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(MaterialData));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    materialData_->color  = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData_->uvTile = uvTile_;

    // WVP 定数バッファ
    wvpResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
    wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
}

void Terrain::Update()
{
    Matrix4x4 world = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    Matrix4x4 wvp   = {};

    if (camera_) {
        wvp = Multiply(world, camera_->GetViewProtectionMatrix());
    } else {
        wvp = Multiply(world, Makeidentity4x4());
    }

    if (wvpData_) {
        wvpData_->WVP   = wvp;
        wvpData_->World = world;
        wvpData_->WorldInverseTranspose = Transpose(Inverse(world));
    }
}

void Terrain::Draw()
{
    if (indices_.empty()) return;

    auto cmdList = DXCommon::GetInstance()->GetCommandList();
    auto psoSet  = PSOManager::GetInstance()->GetPso("Terrain", BlendMode::Normal, FillMode::kSolid);

    cmdList->SetPipelineState(psoSet.pipelineState.Get());
    cmdList->SetGraphicsRootSignature(psoSet.rootSignature.Get());

    cmdList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    cmdList->IASetIndexBuffer(&indexBufferView_);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // [Param 0] Material (CBV b0)
    cmdList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

    // [Param 1] Transform (CBV b1)
    cmdList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

    // [Param 2] Grass Texture (DescriptorTable t0)
    cmdList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(grassTexIndex_));

    // [Param 3] Road Texture (DescriptorTable t1)
    cmdList->SetGraphicsRootDescriptorTable(3, TextureManager::GetInstance()->GetSrvHandleGPU(roadTexIndex_));

    cmdList->DrawIndexedInstanced(static_cast<UINT>(indices_.size()), 1, 0, 0, 0);
}

Terrain::CellType Terrain::GetSurfaceType(const Vector3& worldPos) const
{
    // ワールド座標からローカル座標へ逆変換
    float lx = worldPos.x - transform_.translate.x;
    float lz = worldPos.z - transform_.translate.z;

    float halfX = sizeX_ * 0.5f;
    float halfZ = sizeY_ * 0.5f;

    if (lx < -halfX || lx > halfX || lz < -halfZ || lz > halfZ) {
        return CellType::kGround;
    }

    float tx = (lx + halfX) / sizeX_;
    float tz = (lz + halfZ) / sizeY_;

    int col = std::clamp(static_cast<int>(tx * divX_), 0, divX_ - 1);
    int row = std::clamp(static_cast<int>(tz * divY_), 0, divY_ - 1);
    int idx = row * divX_ + col;

    if (idx >= 0 && idx < static_cast<int>(cellTypes_.size())) {
        return cellTypes_[idx];
    }

    return CellType::kGround;
}
