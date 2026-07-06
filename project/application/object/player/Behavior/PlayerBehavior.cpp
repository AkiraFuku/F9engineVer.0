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
    player->GetState()->BehaviorUpdate(player);
    /* player->RayCastUpdate();
    player->UpdateGravity();*/
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
    if (dynamic_cast<PreShootCommand*>(command)) {
        // ライドオン状態の時のみ照準可能にするチェック
        if (dynamic_cast<IStateRideOn*>(player->GetState())) {
            player->ChangeBehavior(std::make_unique<BehaviorAim>());
        }
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

// PlayerBehavior.cpp

void BehaviorAttack::Update(Player* player) {
    auto attackAction = player->GetState()->GetAttackAction_();

    if (attackAction) {
        // タイマーに応じて速度に倍率をかける（イージング）
        // 最初の数フレームは超高速、後半は急ブレーキ
        float speedMultiplier = 1.0f;

        if (timer_ < 10) {
            speedMultiplier = 2.5f; // 出だしは鋭く！
        } else {
            speedMultiplier = 0.2f; // 後半は反動でゆっくり
        }

        // Action側に倍率を渡せるようにするか、ここでMoveを直接制御する
        int attackDir = player->GetMoveDirection();
        player->Move(float(attackDir) * 0.8f * speedMultiplier);
    }

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
    player->RayCastUpdate();
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
    if (dynamic_cast<PreShootCommand*>(command)) {
        // ライドオン状態の時のみ照準可能にするチェック
        if (dynamic_cast<IStateRideOn*>(player->GetState())) {
            player->ChangeBehavior(std::make_unique<BehaviorAim>());
        }
    }

 
}
// BehaviorAim::Initialize
void BehaviorAim::Initialize(Player* player) {
    // 照準開始時の初期化（必要ならSE再生やエフェクト表示）
    aimX_ = (float)player->GetMoveDirection(); // 現在の向きを初期値に
    aimY_ = 0.0f;
}

void BehaviorAim::Update(Player* player) {
    // 滞空中に照準を定める場合、ゆっくり降下させる
}
void BehaviorAim::Finalize(Player* player)
{}
void BehaviorAim::HandleInput(Player* player, ICommand* command) {
    // 1. AimCommand で方向を更新
    if (auto aimCmd = dynamic_cast<AimCommand*>(command)) {
        // コマンドから入力方向を取得 (例: ジョイスティックのベクトル)
        aimX_ = aimCmd->GetX();
        aimY_ = aimCmd->GetY();
    }

    // 2. ShootCommand (決定) で発射
    if (dynamic_cast<ShootCommand*>(command)) {
        auto state = dynamic_cast<IStateRideOn*>(player->GetState());
        if (state) {
            auto shootAction = state->GetShootAction();
            if (shootAction) {
                // ShootAction側にターゲット方向を渡す仕組みが必要
                static_cast<ShootRobotAction*>(shootAction)->SetAimVector(aimX_, aimY_);
                shootAction->Execute(player);
            }
        }
    
    }

}