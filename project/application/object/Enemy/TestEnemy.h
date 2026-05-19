#pragma once
#include "Enemy.h"
#include <memory>

class TestRobot;
class Player;

/// <summary>
/// テスト用エネミークラス
/// TestRobotを保持し、攻撃時にそれを提供
/// </summary>
class TestEnemy : public Enemy {
public:
    TestEnemy();
    ~TestEnemy();

    void Initialize() override;
    void Update() override;
    void Draw() override;

    // スタン状態時のコールバック
    void OnStun();

  
};
