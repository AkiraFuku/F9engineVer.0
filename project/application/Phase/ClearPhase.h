#pragma once
#include "Phase.h"
#include <memory>
class Sprite;

class ClearPhase :
    public Phase
{
public:
    void Initialize(Scene* scene) override;
    void Update(Scene* scene) override;
    void Draw(Scene* scene) override;
    void Finalize(Scene* scene) override;


private:


    std::unique_ptr<Sprite> clearSprite_;
    bool isTransitioning_ = false;

};

