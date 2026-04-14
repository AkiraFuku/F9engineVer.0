#pragma once
#include <wrl.h>
#include<d3d12.h>
#include<cstdint>
#include <memory>
#include <DirectXTex.h>
class SrvManager
{
public:
    //最大テクスチャ数
    static const uint32_t kMaxSRVCount;
    void Initialize();
    void Finalize();
    // シングルトンインスタンス取得
    static SrvManager* GetInstance();
    friend struct std::default_delete<SrvManager>;
    uint32_t AllocateSRV();
     
    /// <summary>
   /// SRVのCPUディスクリプタハンドルを取得
   /// </summary>
   /// <param name="index"></param>
   /// <returns></returns>
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
    /// <summary>
    /// SRVのGPUディスクリプタハンドルを取得
    /// </summary>
    /// <param name="index"></param>
    /// <returns></returns>
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

    void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DirectX::TexMetadata  metadata);




    void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);

    void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);
    void PreDraw();
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() {
        return descriptorHeap_;
    }

    bool IsMax();
private:


    // シングルトン化に伴いコンストラクタ等をprivateへ
    SrvManager() = default;
    ~SrvManager() = default;

    // コピー禁止
    SrvManager(const SrvManager&) = delete;
    SrvManager& operator=(const SrvManager&) = delete;

    // インスタンス保持用スマートポインタ
  static std::unique_ptr<SrvManager> instance;

    uint32_t descriptorSize_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
    uint32_t useIndex = 0;
};

