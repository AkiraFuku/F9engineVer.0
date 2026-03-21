#pragma once

#include <PhysicsNode.h>
#include <Vector2.h>
#include <Vector4.h>

#include <d3d12.h>
#include <wrl.h>

class Camera;

class ThreadRenderer {
private:
  // 頂点バッファ
  struct VertexData {
    Vector4 position;
    Vector2 texcord;
    Vector3 normal;
  };

  // 行列バッファ
  struct TransformationMatrix {
    Matrix4x4 WVP;
    Matrix4x4 World;
    Matrix4x4 WorldInverseTranspose;
  };

  // マテリアルバッファ(Object3dと共通)
  struct MaterialData {
    Vector4 color;
    int32_t enableLighting;
    float padding[3];
  };

public:
  /// <summary>
  /// 初期化
  /// </summary>
  /// <param name="maxThreads">最大描画本数</param>
  /// <param name="nodesPerThread">1本あたりのノード数</param>
  /// <param name="radius">ロープの太さ</param>
  /// <param name="radialSegments">断面の分割数(6なら六角形)</param>
  void Initialize(int maxThreads, int nodesPerThread, float radius = 0.02f,
                  int radialSegments = 6);

  /// <summary>
  /// 更新
  /// </summary>
  /// <param name="nodes">物理演算のノード</param>
  /// <param name="camera">カメラ</param>
  /// 物理演算のノードを受け取って、頂点バッファをMapして書き換える
  void Update(const std::vector<std::vector<PhysicsNode>> &allThreadNodes,
              Camera *camera);

  /// <summary>
  /// 描画
  /// </summary>
  void Draw();

private:
  float radius_ = 0.5f;
  int radialSegments_ = 6;
  int maxVertices_ = 0;        // 確保した最大頂点数
  int currentVertexCount_ = 0; // 今フレームで実際に描画する頂点数

  // テクスチャハンドル
  uint32_t textureHandle_ = 0;

  // 毎フレームのメモリ確保をなくすための1次元キャッシュ配列
  std::vector<VertexData> ringsCache_;

  // 頂点バッファを持つ
  Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
  D3D12_VERTEX_BUFFER_VIEW vbView_{};

  // 行列をGPUに送るためのバッファ
  Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
  TransformationMatrix *wvpResource_ = nullptr;

  // マテリアルバッファ
  Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
  MaterialData *materialData_ = nullptr;
};