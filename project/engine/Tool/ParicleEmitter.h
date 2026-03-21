#pragma once
#include "Vector4.h"
#include "ParticleManager.h"
#include "Transform.h"
class ParicleEmitter
{
public:
    ParicleEmitter(const  std::string name, EulerTransform transfom, uint32_t count, float frequency, float frequencyTime);
    void Update();
    void Emit();
private:
    EulerTransform transfom_;//位置
    uint32_t count_;//パーティクル数
    float frequency_;//範囲
    float frequencyTime_;//発生時間
    std::string name_;
};

