#include "ThreadRenderer.h"

#include "Camera.h"
#include "DXCommon.h"
#include "MathFunction.h"
#include "TextureManager.h"

/// <summary>
/// 初期化
/// </summary>
/// <param name="maxThreads">最大描画本数</param>
/// <param name="nodesPerThread">1本あたりのノード数</param>
/// <param name="radius">ロープの太さ</param>
/// <param name="radialSegments">断面の分割数(6なら六角形)</param>
void ThreadRenderer::Initialize(int maxThreads, int nodesPerThread,
                                float radius, int radialSegments) {
  radius_ = radius;
  radialSegments_ = radialSegments;

  // ★最大頂点数の計算を変更（全糸の合計）
  int maxSegmentsPerThread = nodesPerThread - 1;
  int verticesPerThread = maxSegmentsPerThread * radialSegments_ * 6;
  maxVertices_ = verticesPerThread * maxThreads;

  // キャッシュ用配列のメモリを事前確保（1本分のリング頂点数）
  ringsCache_.reserve(nodesPerThread * (radialSegments_ + 1));

  // 動的頂点バッファの作成（ご自身のDXCommonの機能を利用！）
  vertexBuffer_ = DXCommon::GetInstance()->CreateBufferResource(
      sizeof(VertexData) * maxVertices_);

  // 頂点バッファビューの設定
  vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
  vbView_.SizeInBytes = sizeof(VertexData) * maxVertices_;
  vbView_.StrideInBytes = sizeof(VertexData);

  // 行列バッファ
  transformationMatrixResource_ = DXCommon::GetInstance()->CreateBufferResource(
      sizeof(TransformationMatrix));
  transformationMatrixResource_.Get()->Map(
      0, nullptr, reinterpret_cast<void **>(&wvpResource_));

  wvpResource_->WVP = Makeidetity4x4();
  wvpResource_->World = Makeidetity4x4();
  wvpResource_->WorldInverseTranspose = Makeidetity4x4();

  // マテリアル用のバッファ
  materialResource_ =
      DXCommon::GetInstance()->CreateBufferResource(sizeof(MaterialData));
  materialResource_.Get()->Map(0, nullptr,
                               reinterpret_cast<void **>(&materialData_));

  // デフォルトの色を白 (R:1, G:1, B:1, A:1) にしておく
  materialData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
  materialData_->enableLighting = 0;

  // 城のテクスチャを読み込んでハンドルに保存
  textureHandle_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(
      "resources/white.png");
}

/// <summary>
/// 更新
/// </summary>
/// <param name="nodes">物理演算のノード</param>
/// <param name="camera">カメラ</param>
/// 物理演算のノードを受け取って、頂点バッファをMapして書き換える
void ThreadRenderer::Update(
    const std::vector<std::vector<PhysicsNode>> &allThreadNodes,
    Camera *camera) {
  VertexData *mappedData = nullptr;
  vertexBuffer_->Map(0, nullptr, reinterpret_cast<void **>(&mappedData));

  int vertexIndex = 0; // 全糸を通した書き込み位置

  // 各糸ごとに処理
  for (const auto &nodes : allThreadNodes) {
    if (nodes.size() < 2)
      continue;

    // 【レベル1の軽量化】配列のクリア（メモリ再確保は起きない）
    ringsCache_.clear();

    // 1. ノードごとにリング状の頂点座標を計算し、キャッシュに詰め込む
    for (size_t i = 0; i < nodes.size(); ++i) {
      Vector3 forward;
      if (i < nodes.size() - 1)
        forward = nodes[i + 1].currentPos - nodes[i].currentPos;
      else
        forward = nodes[i].currentPos - nodes[i - 1].currentPos;

      forward = Normalize(forward);
      Vector3 up = {0.0f, 1.0f, 0.0f};
      if (std::abs(forward.y) > 0.99f)
        up = {1.0f, 0.0f, 0.0f};

      Vector3 right = Normalize(Cross(up, forward));
      Vector3 trueUp = Normalize(Cross(forward, right));

      for (int j = 0; j <= radialSegments_; ++j) {
        float angle =
            (static_cast<float>(j) / radialSegments_) * 3.14159265f * 2.0f;
        Vector3 offset = (right * std::cos(angle)) + (trueUp * std::sin(angle));
        Vector3 normal = Normalize(offset);
        Vector3 pos = nodes[i].currentPos + (normal * radius_);

        VertexData v;
        v.position = {pos.x, pos.y, pos.z, 1.0f};
        v.normal = normal;
        v.texcord = {static_cast<float>(j) / radialSegments_,
                     static_cast<float>(i) / (nodes.size() - 1)};
        ringsCache_.push_back(v);
      }
    }

    // 2. キャッシュから面（ポリゴン）を作って、GPUバッファに一気に書き込む
    int ringSize = radialSegments_ + 1;
    for (size_t i = 0; i < nodes.size() - 1; ++i) {
      for (int j = 0; j < radialSegments_; ++j) {
        if (vertexIndex + 6 > maxVertices_)
          break; // バッファ超過防止

        // 1次元配列なのでアクセスが超高速
        VertexData a = ringsCache_[i * ringSize + j];
        VertexData b = ringsCache_[i * ringSize + j + 1];
        VertexData c = ringsCache_[(i + 1) * ringSize + j];
        VertexData d = ringsCache_[(i + 1) * ringSize + j + 1];

        mappedData[vertexIndex++] = a;
        mappedData[vertexIndex++] = b;
        mappedData[vertexIndex++] = c;
        mappedData[vertexIndex++] = c;
        mappedData[vertexIndex++] = b;
        mappedData[vertexIndex++] = d;
      }
    }
  }

  vertexBuffer_->Unmap(0, nullptr);
  currentVertexCount_ = vertexIndex;

  if (camera) {
    // 糸の頂点はすでにワールド座標なので、World行列は単位行列
    Matrix4x4 worldMatrix = Makeidetity4x4();

    // WVP行列 ＝ (World行列) × (カメラのViewProjection行列)
    // World行列 = 単位行列なので、実質カメラの行列そのまま
    wvpResource_->WVP =
        Multiply(worldMatrix, camera->GetViewProtectionMatrix());
    wvpResource_->World = worldMatrix;
    wvpResource_->WorldInverseTranspose = worldMatrix;
  }
}

/// <summary>
/// 描画
/// </summary>
void ThreadRenderer::Draw() {
  if (currentVertexCount_ == 0) {
    return;
  }

  // マテリアルバッファをセット
  DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(
      0, materialResource_.Get()->GetGPUVirtualAddress());

  // 行列バッファをセット
  DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(
      1, transformationMatrixResource_.Get()->GetGPUVirtualAddress());

  // テクスチャをセット
  DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(
      2, TextureManager::GetInstance()->GetSrvHandleGPU(textureHandle_));

  // 頂点バッファをセット
  DXCommon::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vbView_);
  // 形状を三角形リストに設定
  DXCommon::GetInstance()->GetCommandList()->IASetPrimitiveTopology(
      D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  // 描画実行
  DXCommon::GetInstance()->GetCommandList()->DrawInstanced(currentVertexCount_,
                                                           1, 0, 0);
}