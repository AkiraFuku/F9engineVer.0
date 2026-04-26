#include "PlayerBehavior.h"
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

void BehaviorRoot::Finalize(Player* player)
{
}

void BehaviorRoot::HandleInput(Player* player, ICommand* command) {
    // 移動コマンドの処理
    if (auto moveCmd = dynamic_cast<MoveCommand*>(command)) {
        // 直接 player->Move を呼ばず、Actionを生成して実行
        auto moveAction = std::make_unique<NormalMoveAction>(moveCmd->GetSpeed());
        moveAction->Execute(player);
    } 
    // ジャンプコマンドの処理
    else if (dynamic_cast<JumpCommand*>(command)) {
        // ジャンプ状態へ遷移（遷移後の Initialize で JumpAction が呼ばれる設計を維持）
        player->ChangeBehavior(std::make_unique<BehaviorJump>());
    } 
    else if (dynamic_cast<AttackCommand*>(command)) {
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

void BehaviorAttack::Finalize(Player* player)
{
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
    if (player->IsGround())
    {
        player->ChangeBehavior(std::make_unique<BehaviorRoot>());

    }
}

void BehaviorJump::Finalize(Player* player)
{
}

// --- BehaviorJump (空中状態) ---
void BehaviorJump::HandleInput(Player* player, ICommand* command) {
    // 空中移動の処理もAction経由に統一
    if (auto moveCmd = dynamic_cast<MoveCommand*>(command)) {
        auto moveAction = std::make_unique<NormalMoveAction>(moveCmd->GetSpeed());
        moveAction->Execute(player);
    }
}