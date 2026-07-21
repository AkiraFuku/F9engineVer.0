#pragma once
#include "Enemy.h"
#include "EnemyBehavior.h"
#include "EnemyAction.h"
class TestRobot;
class Player;


class BoundEnemy :
    public Enemy
{


public:
    BoundEnemy();
    ~BoundEnemy()override;

    void Initialize() override;
    void Update() override;
    void Draw() override;
};

class EnemyBehaviorBounce : public IEnemyBehavior {
public:
    EnemyBehaviorBounce() = default;
    ~EnemyBehaviorBounce() override = default;

    void Initialize(Enemy* enemy) override;
    void Update(Enemy* enemy) override;
    void Finalize(Enemy* enemy) override;


  

    const char* GetName() const override {
        return "Bounce";
    }

private:

    std::unique_ptr<IEnemyAction> currentAction_;

    const float kBounceForce = 18.0f; // 跳ね上がる強さ（ゲームに合わせて調整してください）
    const float acceleration_ = 24.0f;    // 前進する速度
};