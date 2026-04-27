#include "EnemyState.h"
#include "Enemy.h"

// --- StateEnemyNormal ---
void StateEnemyNormal::Initialize(Enemy* enemy) {
    // 通常状態の初期化処理
}

void StateEnemyNormal::Update(Enemy* enemy) {

    enemy->UpdateGravity(); // 重力の更新を行う
    // 通常状態の毎フレーム処理
}

void StateEnemyNormal::Finalize(Enemy* enemy) {
    // 通常状態の終了処理
}

// --- StateEnemyStan ---
void StateEnemyStan::Initialize(Enemy* enemy) {
    timer_ = 0.0f;
    // スタン状態開始時の処理（例：移動停止）
}

void StateEnemyStan::Update(Enemy* enemy) {
    timer_ += 1.0f / 60.0f; // 毎フレーム時間を進める（60FPS想定）

    if (timer_ >= kStanDuration) {
        // スタン時間終了後、通常状態へ復帰
        // 注：このメソッド内ではChangeStateを呼ばない
        // 呼び出し側でEnemyの状態遷移を管理すること
    }
}

void StateEnemyStan::Finalize(Enemy* enemy) {
    // スタン状態の終了処理
}

// --- StateEnemyDead ---
void StateEnemyDead::Initialize(Enemy* enemy) {
    // 死亡状態の初期化処理（例：爆発エフェクト再生）
}

void StateEnemyDead::Update(Enemy* enemy) {
    // 死亡状態の毎フレーム処理（例：エフェクト更新、削除タイマー）
}

void StateEnemyDead::Finalize(Enemy* enemy) {
    // 死亡状態の終了処理
}
