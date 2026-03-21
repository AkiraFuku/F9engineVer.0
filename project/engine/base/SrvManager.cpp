#include "SrvManager.h"
#include "DXCommon.h"

std::unique_ptr<SrvManager> SrvManager::instance = nullptr;
const uint32_t SrvManager::kMaxSRVCount = 512;
// インスタンス取得の実装
SrvManager* SrvManager::GetInstance() {
    if (instance == nullptr) {
        // コンストラクタがprivateなのでmake_uniqueではなくnewしてresetする
        instance.reset(new SrvManager());
    }
    return instance.get();
}// 静的メンバ変数の初期化
void SrvManager::Initialize() {
    
   
    descriptorHeap_=DXCommon::GetInstance()->CreateDescriptorHeap( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
    descriptorSize_=DXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}
void SrvManager::Finalize() {
    // インスタンスを破棄（デストラクタが呼ばれ、ComPtrも解放される）
    instance.reset();
}
void SrvManager::PreDraw() {
    ID3D12DescriptorHeap* descritptorHeaps[] = { descriptorHeap_.Get() };
    DXCommon::GetInstance()->GetCommandList()->SetDescriptorHeaps(1, descritptorHeaps);
}
bool SrvManager::IsMax()
{
    if (useIndex > kMaxSRVCount)
    {
        return false;
    }
    return true;

}
uint32_t SrvManager::AllocateSRV() {
    assert(useIndex < kMaxSRVCount);
    int index = useIndex;
    useIndex++;
    return index;
}
D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index) {
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorSize_ * index);
    return handleCPU;
}
D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorSize_ * index);
    return handleGPU;
}
void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels) {
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
    srvDesc.Texture2D.MipLevels = UINT(MipLevels);//最初のミップマップ
    // SRV
    DXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(
        pResource,
        &srvDesc,
        GetCPUDescriptorHandle(srvIndex)
    );
}
void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride) {
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = numElements;
    srvDesc.Buffer.StructureByteStride = structureByteStride;
    D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = GetCPUDescriptorHandle(srvIndex);
    D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = GetGPUDescriptorHandle(srvIndex);
    DXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, instancingSrvHandleCPU);

}
void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex) {
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, GetGPUDescriptorHandle(srvIndex));
}
