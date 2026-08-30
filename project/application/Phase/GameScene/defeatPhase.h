#pragma once
#include "Phase.h"
#include <memory>
#include "Audio.h"
class Sprite;

class defeatPhase :
    public Phase
{
    public:
    void Initialize(Scene* scene) override;
    void Update(Scene* scene) override;
    void Draw(Scene* scene) override;
    void Finalize(Scene* scene) override;


private:

        Audio::SoundHandle defeatSE=0;
    Audio::VoiceHandle Play_=0;
    std::unique_ptr<Sprite> Sprite_;
    bool isTransitioning_ = false;
};

