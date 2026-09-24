#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "Vector4.h"
#include "Vector3.h"

class StageBlock;
class Camera;
class Model;

/// <summary>
/// GPUインスタンスごとのデータ構造（シェーダ側 StructuredBuffer<InstanceData> と完全一致）
/// </summary>
struct BlockInstanceDataGPU {
    Matrix4x4 WVP;
    Matrix4x4 World;
    Matrix4x4 WorldInverseTranspose;
    Vector4 color;
};

/// <summary>
/// 同一モデル・同一テクスチャを持つブロック群を1つのDrawCallで一括描画するレンダラー
/// </summary>
class InstancedBlockRenderer {
public:
    static constexpr uint32_t kMaxInstancesPerBatch = 2048;

    static InstancedBlockRenderer* GetInstance();

    void Initialize();
    void Finalize();

    /// <summary>
    /// フレームの描画準備（インスタンスリストのクリア）
    /// </summary>
    void Begin();

    /// <summary>
    /// ブロックをインスタンス描画キューに追加
    /// </summary>
    void AddBlock(StageBlock* block, Camera* camera);

    /// <summary>
    /// キューに溜まった全ブロックをGPUインスタンシングで一括描画
    /// </summary>
    void Render(Camera* camera);

    /// <summary>
    /// インスタンシング描画の有効/無効フラグ
    /// </summary>
    void SetEnabled(bool enabled) { isEnabled_ = enabled; }
    bool IsEnabled() const { return isEnabled_; }

private:
    InstancedBlockRenderer() = default;
    ~InstancedBlockRenderer() = default;
    InstancedBlockRenderer(const InstancedBlockRenderer&) = delete;
    InstancedBlockRenderer& operator=(const InstancedBlockRenderer&) = delete;

    struct BatchKey {
        std::string modelName;
        std::string texturePath;
        bool operator==(const BatchKey& other) const {
            return modelName == other.modelName && texturePath == other.texturePath;
        }
    };
    struct BatchKeyHasher {
        size_t operator()(const BatchKey& k) const {
            return std::hash<std::string>()(k.modelName) ^ (std::hash<std::string>()(k.texturePath) << 1);
        }
    };

    struct Batch {
        BatchKey key;
        Model* model = nullptr;
        uint32_t textureIndex = 0;
        std::vector<BlockInstanceDataGPU> instances;
    };

    std::unordered_map<BatchKey, Batch, BatchKeyHasher> batches_;

    Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
    BlockInstanceDataGPU* instancingDataMapped_ = nullptr;
    uint32_t instancingSrvIndex_ = 0;

    bool isInitialized_ = false;
    bool isEnabled_ = true;
};
