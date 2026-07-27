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

PrimitiveDrawer *PrimitiveDrawer::GetInstance() {
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
  PsoConfig::ShaderPath vsPath{ShaderType::VS,
                               L"resources/shaders/Primitive/Primitive.vs.hlsl",
                               "main", L"vs_6_0"};
  PsoConfig::ShaderPath psPath{ShaderType::PS,
                               L"resources/shaders/Primitive/Primitive.ps.hlsl",
                               "main", L"ps_6_0"};

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

      Logger::Log(reinterpret_cast<char *>(errorBlob->GetBufferPointer()));
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

  WVPResource_->Map(0, nullptr, reinterpret_cast<void **>(&wvpData_));

  wvpData_->WVP = Makeidentity4x4();
#endif // USE_LINE
}

void PrimitiveDrawer::Initialize() {
#ifdef USE_LINE
  AddPSO();
  // 必要に応じて PointList 等を追加
  WVPResourceCreate();
  // 巨大な頂点バッファを1つ作成
  vertexResource_ = DXCommon::GetInstance()->CreateBufferResource(
      sizeof(VertexData) * kMaxVertices);
  vertexResource_->Map(0, nullptr, reinterpret_cast<void **>(&vertexDataPtr_));

  vbv_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
  vbv_.SizeInBytes = sizeof(VertexData) * kMaxVertices;
  vbv_.StrideInBytes = sizeof(VertexData);

  vertices_.reserve(kMaxVertices);
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

  // --- ここから共通設定 ---

  // 最初の一回だけルートシグネチャを設定（これでバインドが有効になる）
  // ループ内の psoSet
  // から取得しても良いですが、全コマンド共通のはずなので最初の一件から取得
  PsoSet firstPso =
      psoManager->GetPso("Primitive", commands_[0].blendMode,
                         commands_[0].fillMode, commands_[0].psoTopology);
  commandList->SetGraphicsRootSignature(firstPso.rootSignature.Get());

  // ルートシグネチャ設定「後」にCBVをセットする
  commandList->SetGraphicsRootConstantBufferView(
      0, WVPResource_->GetGPUVirtualAddress());
  commandList->IASetVertexBuffers(0, 1, &vbv_);

  // 3. コマンドを順次実行
  for (const auto &cmd : commands_) {
    PsoSet psoSet = psoManager->GetPso("Primitive", cmd.blendMode, cmd.fillMode,
                                       cmd.psoTopology);

    // PSOの切り替え（PSOが同じならスキップする最適化も可能）
    commandList->SetPipelineState(psoSet.pipelineState.Get());

    // 【重要】SetGraphicsRootSignature
    // は基本同じなので、変更がある場合のみ呼ぶ。
    // もし違うルートシグネチャになる可能性があるなら、その後に再度
    // SetGraphicsRootConstantBufferView が必要。 現状はすべて "Primitive"
    // なので、ループ内での SetGraphicsRootSignature は削除してOK。

    commandList->IASetPrimitiveTopology(cmd.topology);
    commandList->DrawInstanced(cmd.vertexCount, 1, cmd.vertexStart, 0);
  }

  // 4. 次フレームのためにクリア
  vertices_.clear();
  commands_.clear();
#endif // USE_LINE
}

void PrimitiveDrawer::DrawLine(const Vector3 &p1, const Vector3 &p2,
                               const Vector4 &color) {
#ifdef USE_LINE
  if (vertices_.size() + 2 > kMaxVertices)
    return;

  DrawCommand cmd;
  cmd.vertexStart = static_cast<uint32_t>(vertices_.size());
  cmd.vertexCount = 2;
  cmd.topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
  cmd.psoTopology = Toporogy::LineList;
  cmd.fillMode = FillMode::kWireFrame; // 線は常にワイヤー
  cmd.blendMode = BlendMode::Normal;

  vertices_.push_back({{p1.x, p1.y, p1.z, 1.0f}, color});
  vertices_.push_back({{p2.x, p2.y, p2.z, 1.0f}, color});

  commands_.push_back(cmd);
#endif // USE_LINE
}
void PrimitiveDrawer::DrawTriangle(const Vector3 &p1, const Vector3 &p2,
                                   const Vector3 &p3, const Vector4 &color,
                                   FillMode fillMode, BlendMode blendMode) {
#ifdef USE_LINE
  // 1. バッファの空き容量チェック (3頂点分)
  if (vertices_.size() + 3 > kMaxVertices) {
    // ロガーなどがあれば警告を出すとデバッグしやすいです
    return;
  }

  // 2. 描画コマンド（予約）を作成
  DrawCommand cmd;
  cmd.vertexStart =
      static_cast<uint32_t>(vertices_.size());        // 現在の末尾を開始位置に
  cmd.vertexCount = 3;                                // 三角形なので3
  cmd.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; // D3D上のトポロジ
  cmd.psoTopology = Toporogy::TriangleList;           // PSO管理用のトポロジ
  cmd.fillMode = fillMode;   // 引数から設定 (Solid / Wireframe)
  cmd.blendMode = blendMode; // 引数から設定 (Normal / Add / etc...)

  // 3. 頂点データを共通バッファに追加
  // VertexData { Vector4 position, Vector4 color }
  vertices_.push_back({{p1.x, p1.y, p1.z, 1.0f}, color});
  vertices_.push_back({{p2.x, p2.y, p2.z, 1.0f}, color});
  vertices_.push_back({{p3.x, p3.y, p3.z, 1.0f}, color});

  // 4. コマンドリストに予約登録
  commands_.push_back(cmd);
#endif // USE_LINE
}
// PrimitiveDrawer.cpp に実装を追加

void PrimitiveDrawer::DrawSphere(const Sphere &sphere, const Vector4 &color) {
#ifdef USE_LINE
  const uint32_t kSubdivision = 16; // 分割数（高くすると滑らかになる）
  const uint32_t kNumVertices = (kSubdivision + 1) * (kSubdivision + 1);
  const uint32_t kNumIndices = kSubdivision * kSubdivision * 6;

  // バッファ溢れチェック
  if (vertices_.size() + kNumIndices > kMaxVertices)
    return;

  // この描画の開始地点を記録
  DrawCommand cmd;
  cmd.vertexStart = static_cast<uint32_t>(vertices_.size());
  cmd.vertexCount = 0; // 後で集計
  cmd.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
  cmd.psoTopology = Toporogy::TriangleList;
  cmd.fillMode = FillMode::kWireFrame; // 通常、デバッグプリミティブはワイヤー
  cmd.blendMode = BlendMode::Normal;

  // 1. 頂点の一時配列を作成
  std::vector<VertexData> gridVertices;
  gridVertices.reserve(kNumVertices);

  for (uint32_t lat = 0; lat <= kSubdivision; ++lat) {
    float phi =
        -std::numbers::pi_v<float> / 2.0f +
        std::numbers::pi_v<float> * static_cast<float>(lat) / kSubdivision;

    for (uint32_t lon = 0; lon <= kSubdivision; ++lon) {
      float theta = 2.0f * std::numbers::pi_v<float> * static_cast<float>(lon) /
                    kSubdivision;

      // ローカル座標での頂点位置
      Vector3 point = {sphere.radius * std::cos(phi) * std::cos(theta),
                       sphere.radius * std::sin(phi),
                       sphere.radius * std::cos(phi) * std::sin(theta)};

      // Quaternionによる回転適用 (RotateVectorは自作の回転関数と想定)
      point = RotateVector(point, sphere.rotate);

      // ワールド座標へ変換
      point = Add(point, sphere.center);

      gridVertices.push_back({{point.x, point.y, point.z, 1.0f}, color});
    }
  }

  // 2. 頂点配列から三角形（インデックス）を構成して vertices_ に追加
  for (uint32_t lat = 0; lat < kSubdivision; ++lat) {
    for (uint32_t lon = 0; lon < kSubdivision; ++lon) {
      uint32_t start = lat * (kSubdivision + 1) + lon;

      // 2つの三角形（1つの四角形ポリゴン）を構成
      // 三角形1
      vertices_.push_back(gridVertices[start]);
      vertices_.push_back(gridVertices[start + 1]);
      vertices_.push_back(gridVertices[start + (kSubdivision + 1)]);

      // 三角形2
      vertices_.push_back(gridVertices[start + 1]);
      vertices_.push_back(gridVertices[start + (kSubdivision + 1) + 1]);
      vertices_.push_back(gridVertices[start + (kSubdivision + 1)]);

      cmd.vertexCount += 6;
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
    Vector4 color = (i == kSubdivision / 2) ? Vector4{0, 0, 0, 1}
                                            : Vector4{0.7f, 0.7f, 0.7f, 1};
  }
#endif // USE_LINE
}

void PrimitiveDrawer::DrawAABB(const AABB& aabb, const Vector4& color) {
    //Vector3 p[8] = {
    //    {aabb.min.x, aabb.min.y, aabb.min.z}, {aabb.max.x, aabb.min.y, aabb.min.z},
    //    {aabb.max.x, aabb.max.y, aabb.min.z}, {aabb.min.x, aabb.max.y, aabb.min.z},
    //    {aabb.min.x, aabb.min.y, aabb.max.z}, {aabb.max.x, aabb.min.y, aabb.max.z},
    //    {aabb.max.x, aabb.max.y, aabb.max.z}, {aabb.min.x, aabb.max.y, aabb.max.z}
    //};

      if (vertices_.size() + 4 > kMaxVertices) return;//

    DrawCommand cmd;
    cmd.vertexStart = static_cast<uint32_t>(vertices_.size());
    cmd.vertexCount = 0;
    cmd.topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    cmd.psoTopology = Toporogy::TriangleList;
    cmd.fillMode = FillMode::kWireFrame; // 線は常にワイヤー
    cmd.blendMode = BlendMode::Normal;

  

    commands_.push_back(cmd);

}

void PrimitiveDrawer::DrawSegment(const Segment &segment,
                                  const Vector4 &color) {
#ifdef USE_LINE
  DrawLine(segment.origin, Add(segment.origin, segment.diff), color);
#endif // USE_LINE
}