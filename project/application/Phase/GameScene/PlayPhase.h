#pragma once
#include "Phase.h"

class GameScene; // 前方宣言

class PlayPhase : public Phase
{
public:
    ~PlayPhase() override = default;
    void Initialize(Scene* scene) override;
    void Update(Scene* scene) override;
    void Draw(Scene* scene) override;
    void Finalize(Scene* scene) override;
};