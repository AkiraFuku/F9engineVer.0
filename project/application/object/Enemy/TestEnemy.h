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

    // テスト用ロボットを取得
    TestRobot* GetRobot() {
        return robot_.get();
    }

    // スタン状態時のコールバック
    void OnStun();

    // プレイヤー攻撃時の衝突処理をオーバーライド
    void OnCollision(Player* other) override;

private:
    std::unique_ptr<TestRobot> robot_;
};
