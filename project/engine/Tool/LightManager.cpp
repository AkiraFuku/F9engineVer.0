#include "LightManager.h"
#include "MathFunction.h" // Normalize関数など
#include "DXCommon.h"
#include <cassert>
// 静的メンバ変数の実体定義
std::unique_ptr<LightManager> LightManager::instance = nullptr;

LightManager* LightManager::GetInstance() {
    if (instance == nullptr) {
        instance.reset(new LightManager());
    }
    return instance.get();
}

void LightManager::Finalize() {
    instance.reset();
}

void LightManager::Initialize() {
    const int kMaxLights = 100;
    CreateStructuredBuffer(sizeof(DirectionalLightData) * kMaxLights, dirLightBuff_);
    CreateStructuredBuffer(sizeof(PointLightData) * kMaxLights, pointLightBuff_);
    CreateStructuredBuffer(sizeof(SpotLightData) * kMaxLights, spotLightBuff_);

    CreateConstBuffer(sizeof(LightCounts), countBuff_);
    countBuff_->Map(0, nullptr, reinterpret_cast<void**>(&countData_));

}

void LightManager::ClearLights() {
    directionalLights_.clear();
    pointLights_.clear();
    spotLights_.clear();
}

LightManager::DirectionalLightData& LightManager::GetDirectionalLight(size_t index)
{
    assert(index < directionalLights_.size() && "DirectionalLight index out of range.");
    return  directionalLights_[index];
}

LightManager::PointLightData& LightManager::GetPointLight(size_t index)
{
    assert(index < pointLights_.size() && "PointLight index out of range.");
    return pointLights_[index];
}

LightManager::SpotLightData& LightManager::GetSpotLight(size_t index)
{

    assert(index < spotLights_.size() && "SpotLight index out of range.");
    return spotLights_[index];
}

void LightManager::SetDirectionalLight(size_t index, const Vector4& color, const Vector3& direction, float intensity) {
    if (index < directionalLights_.size()) {
        directionalLights_[index].color = color;
        directionalLights_[index].direction = direction;
        directionalLights_[index].intensity = intensity;
    }
}

// Point Light Setter
void LightManager::SetPointLight(size_t index, const Vector4& color, const Vector3& position, float intensity, float radius, float decay) {
    if (index < pointLights_.size()) {
        pointLights_[index].color = color;
        pointLights_[index].position = position;
        pointLights_[index].intensity = intensity;
        pointLights_[index].radius = radius;
        pointLights_[index].decay = decay;
    }
}

// Spot Light Setter
void LightManager::SetSpotLight(size_t index, const Vector4& color, const Vector3& position, float intensity, const Vector3& direction, float distance, float decay, float cosAngle, float cosFalloffStart) {
    if (index < spotLights_.size()) {
        spotLights_[index].color = color;
        spotLights_[index].position = position;
        spotLights_[index].intensity = intensity;
        spotLights_[index].direction = direction;
        spotLights_[index].distance = distance;
        spotLights_[index].decay = decay;
        spotLights_[index].cosAngle = cosAngle;
        spotLights_[index].cosFalloffStart = cosFalloffStart;
    }
}


void LightManager::AddDirectionalLight(const Vector4& color, const Vector3& direction, float intensity) {

    DirectionalLightData light;
    light.color = color;
    light.direction = direction;
    light.intensity = intensity;


    directionalLights_.push_back(light);
}
void  LightManager::AddPointLight(const Vector4& color, const Vector3& position, float intensity, float radius, float decay) {
    PointLightData light;
    light.color = color;
    light.position = position;
    light.intensity = intensity;
    light.radius = radius;
    light.decay = decay;
    pointLights_.push_back(light);
}

void LightManager::AddSpotLight(const Vector4& color, const Vector3& position, float intensity, const Vector3& direction, float distance, float decay, float cosAngle, float cosFalloffStart)
{
    SpotLightData light;
    light.color = color;
    light.position = position;
    light.intensity = intensity;
    light.direction = direction;
    light.distance = distance;
    light.decay = decay;
    light.cosAngle = cosAngle;
    light.cosFalloffStart = cosFalloffStart;
    spotLights_.push_back(light);
}



void LightManager::Update() {

    // 1. カウント情報の更新
    countData_->numDirectional = (int)directionalLights_.size();
    countData_->numPoint = (int)pointLights_.size();
    countData_->numSpot = (int)spotLights_.size();
    if (!directionalLights_.empty()) {
        DirectionalLightData* dst = nullptr;
        dirLightBuff_->Map(0, nullptr, reinterpret_cast<void**>(&dst));
        memcpy(dst, directionalLights_.data(), sizeof(DirectionalLightData) * directionalLights_.size());
        dirLightBuff_->Unmap(0, nullptr);
    }
    if (!pointLights_.empty()) {
        PointLightData* dst = nullptr;
        pointLightBuff_->Map(0, nullptr, reinterpret_cast<void**>(&dst));
        memcpy(dst, pointLights_.data(), sizeof(PointLightData) * pointLights_.size());
        pointLightBuff_->Unmap(0, nullptr);
    }
    if (!spotLights_.empty()) {
        SpotLightData* dst = nullptr;
        spotLightBuff_->Map(0, nullptr, reinterpret_cast<void**>(&dst));
        memcpy(dst, spotLights_.data(), sizeof(SpotLightData) * spotLights_.size());
        spotLightBuff_->Unmap(0, nullptr);
    }

}

void LightManager::Draw(UINT rootParameterIndex) {

    // Dir (Index 3)
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootShaderResourceView(rootParameterIndex + 0, dirLightBuff_->GetGPUVirtualAddress());
    // Point (Index 4)
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootShaderResourceView(rootParameterIndex + 1, pointLightBuff_->GetGPUVirtualAddress());
    // Spot (Index 5)
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootShaderResourceView(rootParameterIndex + 2, spotLightBuff_->GetGPUVirtualAddress());
    // Count (Index 6)
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(rootParameterIndex + 3, countBuff_->GetGPUVirtualAddress());
}


void LightManager::CreateStructuredBuffer(size_t sizeInBytes, Microsoft::WRL::ComPtr<ID3D12Resource>& resource) {
    // DXCommonのCreateBufferResourceはConstantBuffer用(256アライメント)になっている場合が多いですが、
    // StructuredBufferもUploadHeapで作るなら基本同じ関数でOKです。
    // ただしサイズは構造体サイズ * 個数でOK（256倍数である必要はないが、あっても問題ない）
    resource = DXCommon::GetInstance()->CreateBufferResource(sizeInBytes);
}

void LightManager::CreateConstBuffer(size_t sizeInBytes, Microsoft::WRL::ComPtr<ID3D12Resource>& resource)
{
    resource = DXCommon::GetInstance()->CreateBufferResource(sizeInBytes);
}
