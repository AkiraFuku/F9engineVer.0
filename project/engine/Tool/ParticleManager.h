#pragma once
#include "Vector4.h"
#include "Vector2.h"
#include<random>
#include<list>
#include "DrawFunction.h"
#include "DXCommon.h"
#include <wrl/client.h>
#include "d3d12.h"
#include <cstdint>
#include "Camera.h"
#include "Transform.h"
class ParticleManager
{
public:
    struct MaterialData {
        std::string textureFilePath;
        uint_fast16_t textureIndex = 0;
    };
    struct VertexData {
        Vector4 position; // 4D position vector
        Vector2 texcoord; // 2D texture coordinate vector
        Vector3 normal;
    };
    struct Material
    {
        Vector4 color;
        int32_t enableLighting;
        float padding[3]; // パディングを追加してサイズを揃える
        Matrix4x4 uvTransform; // UV変換行列
    };
    struct Particle
    {
        EulerTransform transfom;
        Vector3 velocity;
        Vector4 color;
        float lifeTime;
        float currentTime;

    };

    struct ParticleForGPU
    {
        Matrix4x4 WVP;
        Matrix4x4 World;
        Vector4 color;

    };



    struct ParticleGroup {
        MaterialData materialData;
        std::list<Particle> particles;
        uint32_t instancingSrvIndex;
        Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
        ParticleForGPU* instancingData = nullptr;
        uint32_t kNumInstance;
    };


    void Initialize();
    void Update();
    void Draw();
    void CreateParticleGroup(const std::string name, const std::string textureFilepath);
    static ParticleManager* GetInstance();
    void Emit(const std::string name, const Vector3& postion, uint32_t count);
    void Finalize();
    void Setcamera(Camera* camera) {
        camera_ = camera;
    }
    void ReleaseParticleGroup(const std::string name);
    std::unordered_map<std::string, ParticleGroup> particleGroups;
    friend struct std::default_delete<ParticleManager>;
  static std::unique_ptr<ParticleManager> instance;
private:

    ParticleManager() = default;
    ~ParticleManager() = default;
    ParticleManager(ParticleManager&) = delete;
    ParticleManager& operator=(ParticleManager&) = delete;
  
    static uint32_t kMaxNumInstance;
   

    std::random_device seedGen_;
    std::mt19937 randomEngine_;
    HRESULT hr_ = 0;
    //ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature_;
    void CreateRootSignature();
    //グラフィックパイプラインステート
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

    //頂点リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourse_;
    VertexData* vertexData_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
    //マテリアル
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    Material* materialData_ = nullptr;
    void CreateVertexBuffer();
    void CreateMaterialBuffer();
    void CreatePSO();




    Camera* camera_ = nullptr;
};

