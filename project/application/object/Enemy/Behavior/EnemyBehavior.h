#pragma once
#include <memory>
#include "BehaviorState.h"
class Enemy;

class IEnemyBehavior:public IBehavior{
public:
      virtual void Initialize(Enemy* enemy) = 0;
    virtual void Update(Enemy* enemy) = 0;
    virtual void Finalize(Enemy* enemy)=0;
    
};
class EnemyBehaviorPatrol : public IEnemyBehavior {
public:
    void Initialize(Enemy* enemy) override ;
    void Update(Enemy* enemy) override ;
    void Finalize(Enemy* enemy) override ;
    const char* GetName() const override { return "Patrol"; }
};