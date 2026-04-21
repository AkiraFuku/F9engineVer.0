#include "ParicleEmitter.h"
#include "DXCommon.h"

ParticleEmitter::ParticleEmitter(const  std::string name, EulerTransform transfom, uint32_t count, float frequency, float frequencyTime) {
    transform_ = transfom;
    count_ = count;
    frequency_ = frequency;
    frequencyTime_ = frequencyTime;
    name_ = name;

}
void ParticleEmitter::Update() {
    frequencyTime_ += DXCommon::kDeltaTime;
    if (frequency_ <= frequencyTime_)
    {
       ParticleManager::GetInstance()->Emit(name_,transform_.translate,count_);
        frequencyTime_ -= frequency_;

    }
}

void ParticleEmitter::Emit()
{
           ParticleManager::GetInstance()->Emit(name_,transform_.translate,count_);

}
