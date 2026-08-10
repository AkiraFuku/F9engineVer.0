#pragma once
#include <BossPart.h>
class BossWeakPart : public BossPart {
public:
    using BossPart::BossPart;

    void OnCollision(GameObject* other) override ;
};
