#pragma once
#include <memory>
#include "BehaviorState.h"
class Enemy;

class IEnemyBehavior:public IBehavior{
      virtual void Initialize(Enemy* enemy) = 0;
    virtual void Update(Enemy* enemy) = 0;
    virtual void Finalize(Enemy* enemy)=0;
    
};
