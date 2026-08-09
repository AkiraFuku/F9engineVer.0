#include "PrimitiveDrawer.h"

#include "Camera.h"
#include "DXCommon.h"
#include "Logger.h"
#include "MathFunction.h"
#include "PSOManager.h"
#include "RotateFunction.h"
#include <cassert>
#include <numbers>

std::unique_ptr<PrimitiveDrawer> PrimitiveDrawer::instance_ = nullptr;

PrimitiveDrawer* PrimitiveDrawer::GetInstance() {
    if (instance_ == nullptr) {
        // privateコンストラクタを呼び出せるヘルパー構造体
        struct Helper : public PrimitiveDrawer {
            Helper() : PrimitiveDrawer() {}
        };
        instance_ = std::make_unique<Helper>();
    }
    return instance_.get();
}

void PrimitiveDrawer::Finalize() {}

void PrimitiveDrawer::AddPSO() {
#ifdef USE_LINE
    PsoConfig config{};
    PsoConfig::ShaderPath vsPath{ ShaderType::VS,
                                 L"resources/shaders/Primitive/Primitive.vs.hlsl",
                                 "main", L"vs_6_0" };
    PsoConfig::ShaderPath psPath{ ShaderType::PS,
                                 L"resources/shaders/Primitive/Primitive.ps.hlsl",
                                 "main", L"ps_6_0" };

    config.shaderPaths.push_back(vsPath);
    config.shaderPaths.push_back(psPath);

    config.rootSignatureGenerator = []() {
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};

        rootSignatureDesc.Flags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        D3D12_ROOT_PARAMETER rootParameter[1]{};
        rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        rootParameter[0].Descriptor.ShaderRegister = 0; // b0

        rootSignatureDesc.pParameters = rootParameter;
        rootSignatureDesc.NumParameters = _countof(rootParameter);

        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &signatureBlob, &errorBlob);
        if (FAILED(hr)) {
            // エラー処理
            // return Microsoft::WRL::ComPtr<ID3D12RootSignature>();

            Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
            assert(false);
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(
            0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
            IID_PPV_ARGS(&rootSignature));
        if (FAILED(hr)) {
            // エラー処理
            Logger::Log("Failed to create root signature for PrimitiveDrawer.");
            assert(false);
        }

        return rootSignature;
        };

    config.inputLayoutGenerator = []() {
        InputLayout inputLayout = {};
        inputLayout.inputElement = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
             D3D12_APPEND_ALIGNED_ELEMENT,
             D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
             D3D12_APPEND_ALIGNED_ELEMENT,
             D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };

        inputLayout.inputLayout.pInputElementDescs =
            inputLayout.inputElement.data();
        inputLayout.inputLayout.NumElements =
            static_cast<UINT>(inputLayout.inputElement.size());
        return inputLayout;
        };
    //config.depth=
    config.cullMode = D3D12_CULL_MODE_NONE; // カリングなし
    PSOManager::GetInstance()->RegisterPsoGenerator("Primitive", config);
#endif // USE_LINE
}

void PrimitiveDrawer::WVPResourceCreate() {
#ifdef USE_LINE
    WVPResource_ =
        DXCommon::GetInstance()->CreateBufferResource(sizeof(WVPMatrix));

    WVPResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));

    wvpData_->WVP = Makeidentity4x4();
#endif // USE_LINE
}

void PrimitiveDrawer::Initialize() {
#ifdef USE_LINE
    AddPSO();
    WVPResourceCreate();
    // 巨大な頂点バッファを1つ作成
    vertexResource_ = DXCommon::GetInstance()->CreateBufferResource(
        sizeof(VertexData) * kMaxVertices);
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataPtr_));

    vbv_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vbv_.SizeInBytes = sizeof(VertexData) * kMaxVertices;
    vbv_.StrideInBytes = sizeof(VertexData);

    // 2. インデックスバッファ作成（追加）
    indexResource_ = DXCommon::GetInstance()->CreateBufferResource(
        sizeof(uint32_t) * kMaxIndices);
    indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexDataPtr_));

    ibv_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    ibv_.SizeInBytes = sizeof(uint32_t) * kMaxIndices;
    ibv_.Format = DXGI_FORMAT_R32_UINT;


    vertices_.reserve(kMaxVertices);
    indices_.reserve(kMaxIndices);
#endif // USE_LINE
}

void PrimitiveDrawer::Draw() {
#ifdef USE_LINE

    if (commands_.empty())
        return;

    auto commandList = DXCommon::GetInstance()->GetCommandList();
    auto psoManager = PSOManager::GetInstance();

    // 1. WVP更新
    if (camera_) {
        wvpData_->WVP = camera_->GetViewProtectionMatrix();
    } else {
        wvpData_->WVP = Makeidentity4x4();
    }

    // 2. 頂点データをGPUバッファへコピー
    std::memcpy(vertexDataPtr_, vertices_.data(),
        sizeof(VertexData) * vertices_.size());
    std::memcpy(indexDataPtr_, indices_.data(),
        sizeof(uint32_t) * indices_.size());
    // --- ここから共通設定 ---
// 3. ルートシグネチャ・バッファのセット
    PsoSet firstPso =
        psoManager->GetPso("Primitive", commands_[0].blendMode,
            commands_[0].fillMode, commands_[0].psoTopology);
    commandList->SetGraphicsRootSignature(firstPso.rootSignature.Get());
    commandList->SetGraphicsRootConstantBufferView(
        0, WVPResource_->GetGPUVirtualAddress());

    commandList->IASetVertexBuffers(0, 1, &vbv_);
    commandList->IASetIndexBuffer(&ibv_); // ← インデックスバッファのバインドを追加

    // 4. コマンド実行
    for (const auto& cmd : commands_) {
        PsoSet psoSet = psoManager->GetPso("Primitive", cmd.blendMode, cmd.fillMode,
            cmd.psoTopology);
        commandList->SetPipelineState(psoSet.pipelineState.Get());
        commandList->IASetPrimitiveTopology(cmd.topology);

        // DrawInstanced から DrawIndexedInstanced に変更
        commandList->DrawIndexedInstanced(cmd.indexCount, 1, cmd.indexStart,
            cmd.baseVertex, 0);
    }

    // 5. フレーム終わりのクリア
    vertices_.clear();
    indices_.clear();
    commands_.clear();
#endif // USE_LINE
}

void PrimitiveDrawer::DrawLine(const Vector3& p1, const Vector3& p2,
    const Vector4& color) {
#ifdef USE_LINE
    if (vertices_.size() + 2 > kMaxVertices || indices_.size() + 2 > kMaxIndices)
        return;

    DrawCommand cmd;
    cmd.indexStart = static_cast<uint32_t>(indices_.size());
    cmd.indexCount = 2;
    cmd.baseVertex = static_cast<uint32_t>(vertices_.size());
    cmd.topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    cmd.psoTopology = Toporogy::LineList;
    cmd.fillMode = FillMode::kWireFrame;
    cmd.blendMode = BlendMode::Normal;

    // 2頂点を追加
    vertices_.push_back({ {p1.x, p1.y, p1.z, 1.0f}, color });
    vertices_.push_back({ {p2.x, p2.y, p2.z, 1.0f}, color });

    // インデックス（ローカル相対: 0, 1）を追加
    indices_.push_back(0);
    indices_.push_back(1);

    commands_.push_back(cmd);
#endif // USE_LINE
}
void PrimitiveDrawer::DrawTriangle(const Vector3 &p1, const Vector3 &p2,
                                   const Vector3 &p3, const Vector4 &color,
                                   FillMode fillMode, BlendMode blendMode) {
#ifdef USE_LINE
  // 1. バッファの空き容量チェック (3頂点分、3インデックス分)
  if (vertices_.size() + 3 > kMaxVertices || indices_.size() + 3 > kMaxIndices) {
    return;
  }

  // 2. 描画コマンドを作成
  DrawCommand cmd;
  cmd.indexStart = static_cast<uint32_t>(indices_.size());
  cmd.indexCount = 3; // 三角形1つにつきインデックス3つ
  cmd.baseVertex = static_cast<uint32_t>(vertices_.size()); // 現在の頂点末尾を基準に
  cmd.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
  cmd.psoTopology = Toporogy::TriangleList;
  cmd.fillMode = fillMode;
  cmd.blendMode = blendMode;

  // 3. 頂点データを追加
  vertices_.push_back({{p1.x, p1.y, p1.z, 1.0f}, color});
  vertices_.push_back({{p2.x, p2.y, p2.z, 1.0f}, color});
  vertices_.push_back({{p3.x, p3.y, p3.z, 1.0f}, color});

  // 4. インデックスデータを追加（baseVertexからの相対位置: 0, 1, 2）
  indices_.push_back(0);
  indices_.push_back(1);
  indices_.push_back(2);

  // 5. コマンドの登録
  commands_.push_back(cmd);
#endif // USE_LINE
}

void PrimitiveDrawer::DrawSphere(const Sphere& sphere, const Vector4& color) {
#ifdef USE_LINE
    const uint32_t kSubdivision = 16;
    const uint32_t kNumVertices = (kSubdivision + 1) * (kSubdivision + 1);
    const uint32_t kNumIndices = kSubdivision * kSubdivision * 6;

    if (vertices_.size() + kNumVertices > kMaxVertices ||
        indices_.size() + kNumIndices > kMaxIndices)
        return;

    DrawCommand cmd;
    cmd.indexStart = static_cast<uint32_t>(indices_.size());
    cmd.indexCount = kNumIndices;
    cmd.baseVertex = static_cast<uint32_t>(vertices_.size());
    cmd.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    cmd.psoTopology = Toporogy::TriangleList;
    cmd.fillMode = FillMode::kWireFrame;
    cmd.blendMode = BlendMode::Normal;

    // 1. 頂点の生成（重複なしでそのまま追加）
    for (uint32_t lat = 0; lat <= kSubdivision; ++lat) {
        float phi = -std::numbers::pi_v<float> / 2.0f +
            std::numbers::pi_v<float> *static_cast<float>(lat) / kSubdivision;

        for (uint32_t lon = 0; lon <= kSubdivision; ++lon) {
            float theta = 2.0f * std::numbers::pi_v<float> *static_cast<float>(lon) /
                kSubdivision;

            Vector3 point = { sphere.radius * std::cos(phi) * std::cos(theta),
                             sphere.radius * std::sin(phi),
                             sphere.radius * std::cos(phi) * std::sin(theta) };

            point = RotateVector(point, sphere.rotate);
            point = Add(point, sphere.center);

            vertices_.push_back({ {point.x, point.y, point.z, 1.0f}, color });
        }
    }

    // 2. インデックスのみを追加生成
    for (uint32_t lat = 0; lat < kSubdivision; ++lat) {
        for (uint32_t lon = 0; lon < kSubdivision; ++lon) {
            uint32_t start = lat * (kSubdivision + 1) + lon;

            // 三角形1
            indices_.push_back(start);
            indices_.push_back(start + 1);
            indices_.push_back(start + (kSubdivision + 1));

            // 三角形2
            indices_.push_back(start + 1);
            indices_.push_back(start + (kSubdivision + 1) + 1);
            indices_.push_back(start + (kSubdivision + 1));
        }
    }

    commands_.push_back(cmd);
#endif // USE_LINE
}
void PrimitiveDrawer::DrawGrid() {
#ifdef USE_LINE
    const float kGridHalfWidth = 2.0f;
    const uint32_t kSubdivision = 10;
    const float kGridEvery =
        (kGridHalfWidth * 2.0f) / static_cast<float>(kSubdivision);

    for (uint32_t i = 0; i <= kSubdivision; ++i) {
        float pos = -kGridHalfWidth + static_cast<float>(i) * kGridEvery;
        Vector4 color = (i == kSubdivision / 2) ? Vector4{ 0, 0, 0, 1 }
        : Vector4{ 0.7f, 0.7f, 0.7f, 1 };
    }
#endif // USE_LINE
}

void PrimitiveDrawer::DrawAABB(const AABB& aabb, const Vector4& color) {
#ifdef USE_LINE
    if (vertices_.size() + 8 > kMaxVertices || indices_.size() + 24 > kMaxIndices)
        return;

    DrawCommand cmd;
    cmd.indexStart = static_cast<uint32_t>(indices_.size());
    cmd.indexCount = 24; // 12本分（24インデックス）
    cmd.baseVertex = static_cast<uint32_t>(vertices_.size());
    cmd.topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    cmd.psoTopology = Toporogy::LineList;
    cmd.fillMode = FillMode::kWireFrame;
    cmd.blendMode = BlendMode::Normal;

    Vector3 min = aabb.min;
    Vector3 max = aabb.max;

    // 8頂点のみを追加
    vertices_.push_back({ {min.x, min.y, min.z, 1.0f}, color }); // 0
    vertices_.push_back({ {max.x, min.y, min.z, 1.0f}, color }); // 1
    vertices_.push_back({ {max.x, max.y, min.z, 1.0f}, color }); // 2
    vertices_.push_back({ {min.x, max.y, min.z, 1.0f}, color }); // 3
    vertices_.push_back({ {min.x, min.y, max.z, 1.0f}, color }); // 4
    vertices_.push_back({ {max.x, min.y, max.z, 1.0f}, color }); // 5
    vertices_.push_back({ {max.x, max.y, max.z, 1.0f}, color }); // 6
    vertices_.push_back({ {min.x, max.y, max.z, 1.0f}, color }); // 7

    // 12本の線分を結ぶインデックスを指定
    uint32_t boxIndices[24] = {
        // 前面
        0, 1,  1, 2,  2, 3,  3, 0,
        // 後面
        4, 5,  5, 6,  6, 7,  7, 4,
        // 側面（前後を結ぶ）
        0, 4,  1, 5,  2, 6,  3, 7
    };

    indices_.insert(indices_.end(), std::begin(boxIndices), std::end(boxIndices));

    commands_.push_back(cmd);
#endif // USE_LINE
}
void PrimitiveDrawer::DrawSegment(const Segment& segment,
    const Vector4& color) {
#ifdef USE_LINE
    DrawLine(segment.origin, Add(segment.origin, segment.diff), color);
#endif // USE_LINE
}