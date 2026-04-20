#include "BehaviaState.h"
#include "Player.h"
#include "Command.h"
#include "PlayerAction.h"


// --- BehaviorRoot (通常状態) ---
void BehaviorRoot::Initialize(Player* player) {
    // 必要なら初期化処理（アニメーションの切り替えなど）を書く
}

void BehaviorRoot::Update(Player* player) {
    // 毎フレームの処理（特に何もなければ空でもOK）
}

void BehaviorRoot::HandleInput(Player* player, ICommand* command) {
    if (auto move = dynamic_cast<MoveCommand*>(command)) {
        move->Execute(*player);
    } else if (dynamic_cast<JumpCommand*>(command)) {
        player->ChangeBehavior(std::make_unique<BehaviorJump>());
    } else if (dynamic_cast<AttackCommand*>(command)) {
        player->ChangeBehavior(std::make_unique<BehaviorAttack>());
    }
}

// --- BehaviorAttack (攻撃状態) ---
void BehaviorAttack::Initialize(Player* player) {
    timer_ = 0; // 攻撃タイマーをリセット
}

void BehaviorAttack::Update(Player* player) {
    timer_++;
    if (timer_ >= kAttackDuration) {
        player->ChangeBehavior(std::make_unique<BehaviorRoot>());
    }
}

void BehaviorAttack::HandleInput(Player* player, ICommand* command) {
    // 攻撃中は他の入力を受け付けないので空にする
}

// --- BehaviorJump (ジャンプ状態) ---
void BehaviorJump::Initialize(Player* player) {
    auto action = std::make_unique<NormalJumpAction>();
    action->Execute(player);

}

void BehaviorJump::Update(Player* player) {
    // 地面に着いたら Root に戻るなどの判定
    // ※Playerクラスに IsGrounded() などの関数があると便利です
}

void BehaviorJump::HandleInput(Player* player, ICommand* command) {
    if (auto move = dynamic_cast<MoveCommand*>(command)) {
        move->Execute(*player); // ジャンプ中の左右移動は許可
    }
    // JumpCommand は無視（二段ジャンプ禁止の場合）
}