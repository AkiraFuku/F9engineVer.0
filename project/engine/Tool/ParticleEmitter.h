#pragma once
#include "Vector4.h"
#include "ParticleManager.h"
#include "Transform.h"
class ParticleEmitter
{
public:
    ParticleEmitter(const  std::string name, EulerTransform transform, uint32_t count, float frequency, float frequencyTime);
    void SetTransform(const EulerTransform& transform) {
        transform_ = transform;
    }
    void Update();
    void Emit();
private:
    EulerTransform transform_;//位置
    uint32_t count_;//パーティクル数
    float frequency_;//範囲
    float frequencyTime_;//発生時間
    std::string name_;
};

