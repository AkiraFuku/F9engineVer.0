#include "PrimitiveDrawer.h"

#include "DXCommon.h"
#include "PSOManager.h"
#include "Logger.h"
#include <cassert>
#include "MathFunction.h"
#include "Camera.h"
#include "RotateFunction.h"
#include <cstdint>

std::unique_ptr<PrimitiveDrawer> PrimitiveDrawer::instance_ = nullptr;


PrimitiveDrawer* PrimitiveDrawer::GetInstance() {
    if (instance_ == nullptr) {
        // privateコンストラクタを呼び出せるヘルパー構造体
        struct Helper : public PrimitiveDrawer {
            Helper() : PrimitiveDrawer() {
            }
        };
        instance_ = std::make_unique<Helper>();
    }
    return instance_.get();
}

void PrimitiveDrawer::Finalize() {
}



void PrimitiveDrawer::AddPSO()
{
    PsoConfig config{};
    PsoConfig::ShaderPath vsPath{ ShaderType::VS, L"resources/shaders/Primitive/Primitive.vs.hlsl", "main", L"vs_6_0" };
    PsoConfig::ShaderPath psPath{ ShaderType::PS, L"resources/shaders/Primitive/Primitive.ps.hlsl", "main", L"ps_6_0" };

    config.shaderPaths.push_back(vsPath);
    config.shaderPaths.push_back(psPath);

    config.rootSignatureGenerator = []() {








        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};

        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;



        D3D12_ROOT_PARAMETER rootParameter[1]{};
        rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        rootParameter[0].Descriptor.ShaderRegister = 0; // b0

        rootSignatureDesc.pParameters = rootParameter;
        rootSignatureDesc.NumParameters = _countof(rootParameter);




        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        if (FAILED(hr)) {
            // エラー処理
           // return Microsoft::WRL::ComPtr<ID3D12RootSignature>();

            Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
            assert(false);
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        if (FAILED(hr)) {
            // エラー処理
            Logger::Log("Failed to create root signature for PrimitiveDrawer.");
            assert(false);
        }

        return rootSignature;


        };

    config.inputLayoutGenerator = []() {
        return std::vector<D3D12_INPUT_ELEMENT_DESC>{
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        };

    PSOManager::GetInstance()->RegisterPsoGenerator("Primitive", config);

}

void PrimitiveDrawer::WVPResourceCreate()
{

    WVPResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(WVPMatrix));

    WVPResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));

    wvpData_->WVP = Makeidetity4x4();


}

void PrimitiveDrawer::Initialize() {
    AddPSO();
    // 例: 最大頂点数を 100,000 程度に大きく確保

    vertexResource = DXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * kMaxVertices);
    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&sharedMappedPtr_));

    // 各タイプの設定（Topologyの定義のみ）
    auto SetBatch = [&](PrimithiveType type) {

        switch (type)
        {
        case PrimitiveDrawer::PrimithiveType::kLine:
        case PrimitiveDrawer::PrimithiveType::kGrid:
            batches_[type].d3dTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            batches_[type].psoTopology = Toporogy::LineList;
            break;
        case PrimitiveDrawer::PrimithiveType::kTriangle:
        case PrimitiveDrawer::PrimithiveType::kPlane:
        case PrimitiveDrawer::PrimithiveType::kSphere:
        case PrimitiveDrawer::PrimithiveType::kAABB:
            batches_[type].d3dTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            batches_[type].psoTopology = Toporogy::TriangleList;
            break;

        }

        };

    SetBatch(PrimithiveType::kLine);
    SetBatch(PrimithiveType::kTriangle);
    SetBatch(PrimithiveType::kSphere);
    SetBatch(PrimithiveType::kAABB);
    SetBatch(PrimithiveType::kGrid);
    SetBatch(PrimithiveType::kPlane);

    // 必要に応じて PointList 等を追加
    WVPResourceCreate();
}

void PrimitiveDrawer::Draw() {
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    auto psoManager = PSOManager::GetInstance();

    // WVP設定などは共通
    if (camera_) {
        wvpData_->WVP = camera_->GetViewProtectionMatrix();
    }

    // 複製されたコマンドを一つずつ実行
    for (auto& cmd : drawCommands_) {
        auto& batch = batches_[cmd.type];
        if (cmd.vertexCount <= 0) continue;

        // 1. 複製されたデータをGPUへ転送
      // --- オフセットを考慮したVBVを作成 ---
        D3D12_VERTEX_BUFFER_VIEW vbv{};
        vbv.BufferLocation = vertexResource->GetGPUVirtualAddress() + (cmd.startIndex * sizeof(VertexData));
        vbv.SizeInBytes = cmd.vertexCount * sizeof(VertexData);
        vbv.StrideInBytes = sizeof(VertexData);

        // 2. コマンド個別の設定でPSO取得
        PsoSet psoSet = psoManager->GetPso("Primitive", cmd.blendMode, cmd.fillMode, batch.psoTopology);

        commandList->SetPipelineState(psoSet.pipelineState.Get());
        commandList->SetGraphicsRootSignature(psoSet.rootSignature.Get());
        commandList->SetGraphicsRootConstantBufferView(0, WVPResource_->GetGPUVirtualAddress());

        commandList->IASetVertexBuffers(0, 1, &vbv);
        commandList->IASetPrimitiveTopology(batch.d3dTopology);

        // 3. 描画
        commandList->DrawInstanced(static_cast<UINT>(cmd.vertexCount), 1, 0, 0);
    }

    // 全ての描画が終わったらコマンドリストをクリア
    drawCommands_.clear();
    currentVertexOffset_ = 0;
}
void PrimitiveDrawer::DrawLine(const Vector3& p1, const Vector3& p2, const Vector4& color) {
    auto& batch = batches_[PrimithiveType::kLine];
    uint32_t numVertices = 2;
    if (currentVertexOffset_ + numVertices > kMaxVertices) return;
    uint32_t startIndex = currentVertexOffset_;
    sharedMappedPtr_[currentVertexOffset_] = { {p1.x,p1.y,p1.z,1.0f,}, color };
    currentVertexOffset_++;
    sharedMappedPtr_[currentVertexOffset_] = { {p2.x,p2.y,p2.z,1.0f,}, color };
    currentVertexOffset_++;
    drawCommands_.push_back({ PrimithiveType::kLine, numVertices, startIndex });
}

void PrimitiveDrawer::DrawTriangle(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector4& color, FillMode fillMode, BlendMode blendMode)
{
    auto& batch = batches_[PrimithiveType::kTriangle];
    uint32_t numVertices = 3;
    if (currentVertexOffset_ + numVertices > kMaxVertices) return;
    uint32_t startIndex = currentVertexOffset_;


    sharedMappedPtr_[currentVertexOffset_] = { {p1.x,p1.y,p1.z,1.0f,}, color };
    currentVertexOffset_++;
    sharedMappedPtr_[currentVertexOffset_] = { {p2.x,p2.y,p2.z,1.0f,}, color };
    currentVertexOffset_++;
    sharedMappedPtr_[currentVertexOffset_] = { {p3.x,p3.y,p3.z,1.0f,}, color };
    currentVertexOffset_++;

    drawCommands_.push_back({ PrimithiveType::kTriangle, numVertices, startIndex,fillMode,blendMode });

}
// PrimitiveDrawer.cpp に実装を追加

void PrimitiveDrawer::DrawSphere(const Sphere& sphere, const Vector4& color, FillMode fillMode, BlendMode blendMode) {
    auto& batch = batches_[PrimithiveType::kSphere];



    const uint32_t kSubdivision = 16;

    uint32_t numVertices = kSubdivision * kSubdivision * 6;

    // バッファ溢れチェック
    if (currentVertexOffset_ + numVertices > kMaxVertices) return;
    uint32_t startIndex = currentVertexOffset_;
    const float kLonEvery = 2.0f * PI / static_cast<float>(kSubdivision);
    const float kLatEvery = PI / static_cast<float>(kSubdivision);

    Quaternion normRotate = Normalize(sphere.rotate);

    for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
        float lat = -PI / 2.0f + kLatEvery * latIndex;
        for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
            float lon = lonIndex * kLonEvery;

            // --- 4つの頂点を計算 ---
            auto GetPos = [&](float phi, float theta) {
                Vector3 p = {
                    sphere.radius * cosf(phi) * cosf(theta),
                    sphere.radius * sinf(phi),
                    sphere.radius * cosf(phi) * sinf(theta)
                };
                return Add(RotateVector(p, normRotate), sphere.center);
                };

            Vector3 p1 = GetPos(lat, lon);                      // 左下
            Vector3 p2 = GetPos(lat + kLatEvery, lon);          // 左上
            Vector3 p3 = GetPos(lat, lon + kLonEvery);          // 右下
            Vector3 p4 = GetPos(lat + kLatEvery, lon + kLonEvery); // 右上

            //if (batch.vertices.size() + 6 > kMaxVertices) return;


            sharedMappedPtr_[currentVertexOffset_] = { {p1.x, p1.y, p1.z, 1.0f}, color };
            currentVertexOffset_++;
            sharedMappedPtr_[currentVertexOffset_] = { {p2.x, p2.y, p2.z, 1.0f}, color };
            currentVertexOffset_++;
            sharedMappedPtr_[currentVertexOffset_] = { {p3.x, p3.y, p3.z, 1.0f}, color };
            currentVertexOffset_++;

            sharedMappedPtr_[currentVertexOffset_] = { {p2.x, p2.y, p2.z, 1.0f}, color };
            currentVertexOffset_++;
            sharedMappedPtr_[currentVertexOffset_] = { {p4.x, p4.y, p4.z, 1.0f}, color };
            currentVertexOffset_++;
            sharedMappedPtr_[currentVertexOffset_] = { {p3.x, p3.y, p3.z, 1.0f}, color };
            currentVertexOffset_++;

            //// --- 三角形1 (左下、左上、右下) ---
            //batch.vertices.push_back({ {p1.x, p1.y, p1.z, 1.0f}, color });
            //batch.vertices.push_back({ {p2.x, p2.y, p2.z, 1.0f}, color });
            //batch.vertices.push_back({ {p3.x, p3.y, p3.z, 1.0f}, color });

            //// --- 三角形2 (左上、右上、右下) ---
            //batch.vertices.push_back({ {p2.x, p2.y, p2.z, 1.0f}, color });
            //batch.vertices.push_back({ {p4.x, p4.y, p4.z, 1.0f}, color });
            //batch.vertices.push_back({ {p3.x, p3.y, p3.z, 1.0f}, color });
        }
    }
    //DrawCommand cmd;
    //cmd.type = PrimithiveType::kSphere;
    //cmd.vertices = batch.vertices; // 頂点ベクタをまるごとコピー
    //cmd.fillMode = fillMode;
    //cmd.blendMode = blendMode;
    //drawCommands_.push_back(std::move(cmd));
    //batch.vertices.clear();
	drawCommands_.push_back({ PrimithiveType::kSphere, numVertices, startIndex, fillMode, blendMode });
}
// --- DrawGrid の修正（グリッド専用バッチに直接代入） ---
void PrimitiveDrawer::DrawGrid() {
/*    auto& batch = batches_[PrimithiveType::kGrid];
    const float kGridHalfWidth = 2.0f;
    const uint32_t kSubdivision = 10;
    const float kGridEvery = (kGridHalfWidth * 2.0f) / static_cast<float>(kSubdivision);

    for (uint32_t i = 0; i <= kSubdivision; ++i) {
        float pos = -kGridHalfWidth + static_cast<float>(i) * kGridEvery;
        Vector4 color = (i == kSubdivision / 2) ? Vector4{ 0,0,0,1 } : Vector4{ 0.7f, 0.7f, 0.7f, 1 };

        if (batch.vertices.size() + 4 > kMaxVertices) break;

        // X方向の線
        batch.vertices.push_back({ {pos, 0, kGridHalfWidth, 1.0f}, color });
        batch.vertices.push_back({ {pos, 0, -kGridHalfWidth, 1.0f}, color });
        // Z方向の線
        batch.vertices.push_back({ {kGridHalfWidth, 0, pos, 1.0f}, color });
        batch.vertices.push_back({ {-kGridHalfWidth, 0, pos, 1.0f}, color });
    }

    DrawCommand cmd;
    cmd.type = PrimithiveType::kGrid;
    cmd.vertices = batch.vertices; // 頂点ベクタをまるごとコピー
    drawCommands_.push_back(std::move(cmd));
    batch.vertices.clear();*/
}

// --- DrawAABB の修正（AABB専用バッチに直接代入） ---
void PrimitiveDrawer::DrawAABB(const AABB& aabb, const Vector4& color) {
/*    auto& batch = batches_[PrimithiveType::kAABB];
    if (batch.vertices.size() + 24 > kMaxVertices) return; // 線12本 = 24頂点

    Vector3 p[8] = {
        {aabb.min.x, aabb.min.y, aabb.min.z}, {aabb.max.x, aabb.min.y, aabb.min.z},
        {aabb.max.x, aabb.max.y, aabb.min.z}, {aabb.min.x, aabb.max.y, aabb.min.z},
        {aabb.min.x, aabb.min.y, aabb.max.z}, {aabb.max.x, aabb.min.y, aabb.max.z},
        {aabb.max.x, aabb.max.y, aabb.max.z}, {aabb.min.x, aabb.max.y, aabb.max.z}
    };

    // インデックス順に頂点を流し込む（LineList想定）
    auto AddLine = [&](int i1, int i2) {
        batch.vertices.push_back({ {p[i1].x, p[i1].y, p[i1].z, 1.0f}, color });
        batch.vertices.push_back({ {p[i2].x, p[i2].y, p[i2].z, 1.0f}, color });
        };

    AddLine(0, 1); AddLine(1, 2); AddLine(2, 3); AddLine(3, 0); // 底面
    AddLine(4, 5); AddLine(5, 6); AddLine(6, 7); AddLine(7, 4); // 上面
    AddLine(0, 4); AddLine(1, 5); AddLine(2, 6); AddLine(3, 7); // 柱

    DrawCommand cmd;
    cmd.type = PrimithiveType::kAABB;
    cmd.vertices = batch.vertices; // 頂点ベクタをまるごとコピ-
    drawCommands_.push_back(std::move(cmd));
    batch.vertices.clear();*/
}

void PrimitiveDrawer::DrawSegment(const Segment& segment, const Vector4& color) {
    DrawLine(segment.origin, Add(segment.origin, segment.diff), color);
}