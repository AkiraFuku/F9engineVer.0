#include "PlayerBehavior.h"
#include "Player.h"
#include "Command.h"
#include "PlayerAction.h"
#include "PlayerState.h"

// --- BehaviorRoot (通常状態) ---
void BehaviorRoot::Initialize(Player* player) {
    // 必要なら初期化処理（アニメーションの切り替えなど）を書く
}

void BehaviorRoot::Update(Player* player) {
    // 毎フレームの処理（特に何もなければ空でもOK）
    player->UpdateGravity();
}

void BehaviorRoot::Finalize(Player* player)
{
}

void BehaviorRoot::HandleInput(Player* player, ICommand* command) {
   auto moveAction = player->GetState()->GetMoveAction();

    if (auto moveCmd = dynamic_cast<MoveCommand*>(command)) {
        if (moveAction) {
            // コマンドから速度を抽出し、アクションにセットして実行
            // ※Action側にパラメータセット用メソッド(SetSpeed等)が必要
            static_cast<NormalMoveAction*>(moveAction)->SetSpeed(moveCmd->GetSpeed());
            moveAction->Execute(player);
        }
    } 
    
    if (dynamic_cast<JumpCommand*>(command)) {
        // ジャンプ状態へ遷移
        player->ChangeBehavior(std::make_unique<BehaviorJump>());
    }
     if (dynamic_cast<AttackCommand*>(command)) {
        player->ChangeBehavior(std::make_unique<BehaviorAttack>());
    }
}

// --- BehaviorAttack (攻撃状態) ---
void BehaviorAttack::Initialize(Player* player) {
    timer_ = 0; // 攻撃タイマーをリセット
    timer_ = 0;
    // Stateから攻撃アクションを取得して実行
    auto attackAction = player->GetState()->GetAttackAction_();
    if (attackAction) {
        attackAction->Execute(player);
    }
}

void BehaviorAttack::Update(Player* player) {
   auto attackAction = player->GetState()->GetAttackAction_();
    if (attackAction) {
        attackAction->Execute(player);
    }

    timer_++;
    if (timer_ >= kAttackDuration) {
        // 一定時間（kAttackDuration）経ったら通常状態に戻る
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
  auto jumpAction = player->GetState()->GetJumpAction();
    if (jumpAction) {
        jumpAction->Execute(player);
    }


}

void BehaviorJump::Update(Player* player) {
    // 地面に着いたら Root に戻るなどの判定
    // ※Playerクラスに IsGrounded() などの関数があると便利です
    if (player->IsGround())
    {
        player->ChangeBehavior(std::make_unique<BehaviorRoot>());

    }
       player->UpdateGravity();
}

void BehaviorJump::Finalize(Player* player)
{
}

// --- BehaviorJump (空中状態) ---
void BehaviorJump::HandleInput(Player* player, ICommand* command) {
  auto moveAction = player->GetState()->GetMoveAction();

    if (auto moveCmd = dynamic_cast<MoveCommand*>(command)) {
        if (moveAction) {
            // コマンドから速度を抽出し、アクションにセットして実行
            // ※Action側にパラメータセット用メソッド(SetSpeed等)が必要
            static_cast<NormalMoveAction*>(moveAction)->SetSpeed(moveCmd->GetSpeed());
            moveAction->Execute(player);
        }
    } 
    if (dynamic_cast<AttackCommand*>(command)) {
        // 攻撃状態へ遷移
        player->ChangeBehavior(std::make_unique<BehaviorAttack>());
    }
    
/*    if (dynamic_cast<JumpCommand*>(command)) {
        // ジャンプ状態へ遷移
        player->ChangeBehavior(std::make_unique<BehaviorJump>());
    }*/
}