#pragma once
#include "BossPart.h"
class BossBodyPart : public BossPart {
public:
    using BossPart::BossPart;

    void OnCollision(GameObject* other) override ;
};