#pragma once
#include "Phase.h"

class StartPhase : public Phase
{
public:
    ~StartPhase() override = default;

    void Initialize(Scene* scene) override;
    void Update(Scene* scene) override;
    void Draw(Scene* scene) override;
    void Finalize(Scene* scene) override;

private:
    const float kFadeDuration_ = 5.0f; // フェードインにかける時間（秒）
};