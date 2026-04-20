#include "PrimitiveDrawer.h"

#include "DXCommon.h"
#include "PSOManager.h"
#include "Logger.h"
#include <cassert>
#include "MathFunction.h"
#include "Camera.h"
#include "RotateFunction.h"

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
    auto CreateBatch = [&](PrimtiveType type, D3D_PRIMITIVE_TOPOLOGY d3dTop, Toporogy psoTop) {
        PrimitiveBatch batch;
        batch.d3dTopology = d3dTop;
        batch.psoTopology = psoTop;

        // リソース作成
        batch.resource = DXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * kMaxVertices);

        // VBV設定
        batch.vbv.BufferLocation = batch.resource->GetGPUVirtualAddress();
        batch.vbv.SizeInBytes = sizeof(VertexData) * kMaxVertices;
        batch.vbv.StrideInBytes = sizeof(VertexData);

        //batch.vertices.reserve(kMaxVertices);

        batch.fillMode = FillMode::kWireFrame;
        batch.blendMode = BlendMode::Normal;
        batches_[type] = std::move(batch);
        };

    CreateBatch(PrimtiveType::kLine, D3D_PRIMITIVE_TOPOLOGY_LINELIST, Toporogy::LineList);
    CreateBatch(PrimtiveType::kTriangle, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, Toporogy::TriangleList);
    CreateBatch(PrimtiveType::kSpher, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, Toporogy::TriangleList);
    // 必要に応じて PointList 等を追加
    WVPResourceCreate();
}

void PrimitiveDrawer::Draw() {
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    auto psoManager = PSOManager::GetInstance();
    if (camera_) {
        wvpData_->WVP = camera_->GetViewProtectionMatrix();
    } else {
        wvpData_->WVP = Makeidetity4x4();
    }
    for (auto& [type, batch] : batches_) {
        if (batch.vertices.empty()) continue;

        // 1. このトポロジ専用のリソースにデータをコピー
        void* mappedPtr = nullptr;
        batch.resource->Map(0, nullptr, &mappedPtr);
        std::memcpy(mappedPtr, batch.vertices.data(), sizeof(VertexData) * batch.vertices.size());
        batch.resource->Unmap(0, nullptr);

        // 2. PSOの取得と設定
        // FillModeなどは必要に応じて引数化してください
        PsoSet psoSet = psoManager->GetPso("Primitive", batch.blendMode, batch.fillMode, batch.psoTopology);

        commandList->SetPipelineState(psoSet.pipelineState.Get());
        commandList->SetGraphicsRootSignature(psoSet.rootSignature.Get());

        // ★追加: WVP行列の定数バッファ(CBV)をコマンドリストにセット (ルートパラメータインデックス0番)
        commandList->SetGraphicsRootConstantBufferView(0, WVPResource_->GetGPUVirtualAddress());

        // 3. このトポロジ専用のVBVをセット
        commandList->IASetVertexBuffers(0, 1, &batch.vbv);
        commandList->IASetPrimitiveTopology(batch.d3dTopology);

        // 4. 描画
        commandList->DrawInstanced(static_cast<UINT>(batch.vertices.size()), 1, 0, 0);

        // 5. 次のフレームのためにクリア
        batch.vertices.clear();
    }
}
void PrimitiveDrawer::DrawLine(const Vector3& p1, const Vector3& p2, const Vector4& color) {
    auto& batch = batches_[PrimtiveType::kLine];
    if (batch.vertices.size() + 2 > kMaxVertices) return;

    batch.vertices.push_back({ {p1.x, p1.y, p1.z, 1.0f}, color });
    batch.vertices.push_back({ {p2.x, p2.y, p2.z, 1.0f}, color });

}

void PrimitiveDrawer::DrawTriangle(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector4& color, FillMode fillMode, BlendMode blendMode)
{
    auto& batch = batches_[PrimtiveType::kTriangle];
    if (batch.vertices.size() + 3 > kMaxVertices) return;
    batch.fillMode = fillMode;
    batch.blendMode = blendMode;

    batch.vertices.push_back({ {p1.x, p1.y, p1.z, 1.0f}, color });
    batch.vertices.push_back({ {p2.x, p2.y, p2.z, 1.0f}, color });
    batch.vertices.push_back({ {p3.x, p3.y, p3.z, 1.0f}, color });


}
// PrimitiveDrawer.cpp に実装を追加

void PrimitiveDrawer::DrawSphere(const Sphere& sphere, const Vector4& color) {
    auto& batch = batches_[PrimtiveType::kSpher];

    // 分割数（精度）の設定
    const uint32_t kSubdivision = 16;
    // 1つの球に必要な頂点数：分割数 * 分割数 * 6 (2トライアングル)
    if (batch.vertices.size() + (kSubdivision * kSubdivision * 6) > kMaxVertices) return;

    const float kLonEvery = 2.0f * PI / static_cast<float>(kSubdivision);
    const float kLatEvery = PI / static_cast<float>(kSubdivision);

    Quaternion normRotate = Normalize(sphere.rotate);

    for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
        // 現在の緯度と次の緯度 (-PI/2 ～ PI/2)
        float lat0 = -PI / 2.0f + kLatEvery * latIndex;
        float lat1 = -PI / 2.0f + kLatEvery * (latIndex + 1);

        for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
            // 現在の経度と次の経度 (0 ～ 2PI)
            float lon0 = lonIndex * kLonEvery;
            float lon1 = (lonIndex + 1) * kLonEvery;

            // 四角形の4頂点を計算 (A, B, C, D)
            auto GetPos = [&](float lat, float lon) {
                Vector3 local = {
                    sphere.radius * cosf(lat) * cosf(lon),
                    sphere.radius * sinf(lat),
                    sphere.radius * cosf(lat) * sinf(lon)
                };
                // 回転と平行移動
                Vector3 rotated = RotateVector(local, normRotate);
                return Add(rotated, sphere.center);
                };

            Vector3 v0 = GetPos(lat0, lon0); // 左下
            Vector3 v1 = GetPos(lat1, lon0); // 左上
            Vector3 v2 = GetPos(lat0, lon1); // 右下
            Vector3 v3 = GetPos(lat1, lon1); // 右上

            // 三角形1 (左下、左上、右下)
            batch.vertices.push_back({ {v0.x, v0.y, v0.z, 1.0f}, color });
            batch.vertices.push_back({ {v1.x, v1.y, v1.z, 1.0f}, color });
            batch.vertices.push_back({ {v2.x, v2.y, v2.z, 1.0f}, color });

            // 三角形2 (左上、右上、右下)
            batch.vertices.push_back({ {v1.x, v1.y, v1.z, 1.0f}, color });
            batch.vertices.push_back({ {v3.x, v3.y, v3.z, 1.0f}, color });
            batch.vertices.push_back({ {v2.x, v2.y, v2.z, 1.0f}, color });
        }
    }
}
void PrimitiveDrawer::DrawGrid() {
    const float kGridHalfWidth = 2.0f;
    const uint32_t kSubdivision = 10;
    const float kGridEvery = (kGridHalfWidth * 2.0f) / static_cast<float>(kSubdivision);

    for (uint32_t i = 0; i <= kSubdivision; ++i) {
        float pos = -kGridHalfWidth + static_cast<float>(i) * kGridEvery;
        Vector4 color = (i == kSubdivision / 2) ? Vector4{ 0,0,0,1 } : Vector4{ 0.7f, 0.7f, 0.7f, 1 };

        // X方向の線
        DrawLine({ pos, 0, kGridHalfWidth }, { pos, 0, -kGridHalfWidth }, color);
        // Z方向の线
        DrawLine({ kGridHalfWidth, 0, pos }, { -kGridHalfWidth, 0, pos }, color);
    }
}

void PrimitiveDrawer::DrawAABB(const AABB& aabb, const Vector4& color) {
    Vector3 p[8] = {
        {aabb.min.x, aabb.min.y, aabb.min.z}, {aabb.max.x, aabb.min.y, aabb.min.z},
        {aabb.max.x, aabb.max.y, aabb.min.z}, {aabb.min.x, aabb.max.y, aabb.min.z},
        {aabb.min.x, aabb.min.y, aabb.max.z}, {aabb.max.x, aabb.min.y, aabb.max.z},
        {aabb.max.x, aabb.max.y, aabb.max.z}, {aabb.min.x, aabb.max.y, aabb.max.z}
    };

    // 底面
    DrawLine(p[0], p[1], color); DrawLine(p[1], p[2], color);
    DrawLine(p[2], p[3], color); DrawLine(p[3], p[0], color);
    // 上面
    DrawLine(p[4], p[5], color); DrawLine(p[5], p[6], color);
    DrawLine(p[6], p[7], color); DrawLine(p[7], p[4], color);
    // 柱
    DrawLine(p[0], p[4], color); DrawLine(p[1], p[5], color);
    DrawLine(p[2], p[6], color); DrawLine(p[3], p[7], color);
}

void PrimitiveDrawer::DrawSegment(const Segment& segment, const Vector4& color) {
    DrawLine(segment.origin, Add(segment.origin, segment.diff), color);
}