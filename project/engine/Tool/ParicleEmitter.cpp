#include "ParicleEmitter.h"
#include "DXCommon.h"

ParicleEmitter::ParicleEmitter(const  std::string name, EulerTransform transfom, uint32_t count, float frequency, float frequencyTime) {
    transfom_ = transfom;
    count_ = count;
    frequency_ = frequency;
    frequencyTime_ = frequencyTime;
    name_ = name;

}
void ParicleEmitter::Update() {
    frequencyTime_ += DXCommon::kDeltaTime;
    if (frequency_ <= frequencyTime_)
    {
       ParticleManager::GetInstance()->Emit(name_,transfom_.translate,count_);
        frequencyTime_ -= frequency_;

    }
}

void ParicleEmitter::Emit()
{
           ParticleManager::GetInstance()->Emit(name_,transfom_.translate,count_);

}
