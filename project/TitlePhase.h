#pragma once
#include "Phase.h"
class TitlePhase :
    public Phase
{
public:
    ~TitlePhase() override = default;
    void Initialize(Scene* scene) override;
    void Update(Scene* scene) override;
    void Draw(Scene* scene) override;
    void Finalize(Scene* scene) override;
private:
    bool isTransitioning_=false;


};

