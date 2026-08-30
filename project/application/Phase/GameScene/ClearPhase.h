#pragma once
#include "Phase.h"
#include"Audio.h"

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
    Audio::SoundHandle ClearSE=0;
    Audio::VoiceHandle Play_=0;

    std::unique_ptr<Sprite> clearSprite_;
    bool isTransitioning_ = false;

};

