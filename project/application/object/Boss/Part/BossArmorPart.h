#pragma once
#include "BossPart.h"
class BossArmorPart :
    public BossPart
{public:
    using BossPart::BossPart;

    void OnCollision(GameObject* other) override;
};