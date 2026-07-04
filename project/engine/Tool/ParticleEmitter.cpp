#include "ParticleEmitter.h"
#include "DXCommon.h"

ParticleEmitter::ParticleEmitter(
    const std::string& setName,
    const std::string& groupName,
    EulerTransform transform,
    uint32_t count,
    float frequency,
    float frequencyTime)
{
    setName_ = setName;
    groupName_ = groupName;
    transform_ = transform;
    count_ = count;
    frequency_ = frequency;
    frequencyTime_ = frequencyTime;
}

void ParticleEmitter::Update()
{
    frequencyTime_ += DXCommon::kDeltaTime;
    if (frequency_ <= frequencyTime_)
    {
        ParticleManager::GetInstance()->Emit(setName_, groupName_, transform_.translate, count_);
        frequencyTime_ -= frequency_;
    }
}

void ParticleEmitter::Emit()
{
    ParticleManager::GetInstance()->Emit(setName_, groupName_, transform_.translate, count_);
}
