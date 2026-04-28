#include "TestEnemy.h"
#include "TestRobot.h"
#include "Player.h"
#include "PlayerState.h"
#include "EnemyState.h"

TestEnemy::TestEnemy() {
}

TestEnemy::~TestEnemy() = default;

void TestEnemy::Initialize() {
    Enemy::Initialize();

    // テスト用ロボットを初期化
    robot_ = std::make_unique<TestRobot>();
    // Robot基底クラスはObject3dを持たないため、Initializeは呼び出さない
}

void TestEnemy::Update() {
    Enemy::Update();

    // ロボット自体は更新が必要ないため、スキップ
}

void TestEnemy::Draw() {
    Enemy::Draw();

    // ロボット自体は描画が必要ないため、スキップ
}

void TestEnemy::OnStun() {
    // スタン状態になったときの処理
    // 必要に応じてオーバーライド可能
}

void TestEnemy::OnCollision(Player* other) {
    if (!other) return;

    if (GetStateName() && strcmp(GetStateName(), "Dead") == 0) {
        return; // すでに死んでいるなら何もしない
    }

    // プレイヤーの状態を取得
    const char* playerBehavior = other->GetBehaviorName();
    const char* playerState = other->GetStateName();

    if (playerState && strcmp(playerState, "Normal") == 0) {
        if (playerBehavior && strcmp(playerBehavior, "Attack") == 0) {

            const char* enemyState = GetStateName();

            if (enemyState && strcmp(enemyState, "Normal") == 0) {
                // Normal 状態から Stan 状態へ遷移
                // クールダウンを設定して次の攻撃を防ぐ
                isHit_ = true;
                hitVisualTimer_ = 1.0f;
                ChangeState(std::make_unique<StateEnemyStan>());
            } else if (enemyState && strcmp(enemyState, "Stan") == 0) {
                // Stan 状態から攻撃されたので、ロボットの状態をプレイヤーに設定
                if (robot_) {
                    // ロボットのPlayerState_を取得して、プレイヤーに設定
                    if (robot_->PlayerState_) {
                        // PlayerState_を複製して新しい状態として設定
                        other->ChangeState(std::make_unique<StateRideOnTest>());
                    }
                }
                ChangeState(std::make_unique<StateEnemyDead>());
            }
        }
    }
}



