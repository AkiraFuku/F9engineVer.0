#include "PlayerBehavior.h"
#include "Player.h"
#include "Command.h"
#include "PlayerAction.h"
#include "PlayerState.h"
#include "IPlayerFactory.h" // ★ Factory のヘッダーをインクルード

// --- Helper 関数（State から Factory を安全に取得） ---
static IPlayerFactory* GetFactoryFromPlayer(Player* player) {
    auto state = player->GetState();
    return state ? state->GetFactory() : nullptr;
}

// --- BehaviorRoot ---
void BehaviorRoot::Initialize(Player* player) {}

void BehaviorRoot::Update(Player* player) {
    player->RayCastUpdate();
    player->UpdateGravity();
}

void BehaviorRoot::Finalize(Player* player) {}

void BehaviorRoot::HandleInput(Player* player, ICommand* command) {
    auto state = player->GetState();
    if (!state) return;

    auto moveAction = state->GetMoveAction();

    if (auto moveCmd = dynamic_cast<MoveCommand*>(command)) {
        if (moveAction) {
            static_cast<NormalMoveAction*>(moveAction)->SetSpeed(moveCmd->GetSpeed());
            moveAction->Execute(player);
        }
    }

    // ★ Factory を取得して生成するように変更
    auto factory = state->GetFactory();
    if (!factory) return;

    if (dynamic_cast<JumpCommand*>(command)) {
        state->ChangeBehavior(player, factory->CreateBehavior(BehaviorType::Jump));
    }
    if (dynamic_cast<AttackCommand*>(command)) {
        state->ChangeBehavior(player, factory->CreateBehavior(BehaviorType::Attack));
    }
    if (dynamic_cast<PreShootCommand*>(command)) {
        if (dynamic_cast<IStateRideOn*>(state)) {
            state->ChangeBehavior(player, factory->CreateBehavior(BehaviorType::Aim));
        }
    }
}

// --- BehaviorAttack ---
void BehaviorAttack::Initialize(Player* player) {
    timer_ = 0.0f;
    auto state = player->GetState();
    if (state) {
        auto attackAction = state->GetAttackAction_();
        if (attackAction) {
            attackAction->Execute(player);
        }
    }
}

void BehaviorAttack::Update(Player* player) {
    auto state = player->GetState();
    if (state) {
        auto attackAction = state->GetAttackAction_();
        if (attackAction) {
            float speedMultiplier = (timer_ < 0.15f) ? 2.5f : 0.2f;
            int attackDir = player->GetMoveDirection();
            player->Move(float(attackDir) * 0.8f * speedMultiplier);
        }
    }

    timer_ += player->GetDeltaTime();
    if (timer_ >= kAttackDuration) {
        if (state) {
            // ★ Factory 経由で Root に戻る
            if (auto factory = state->GetFactory()) {
                state->ChangeBehavior(player, factory->CreateBehavior(BehaviorType::Root));
            }
        }
    }
}

void BehaviorAttack::Finalize(Player* player) {}
void BehaviorAttack::HandleInput(Player* player, ICommand* command) {}

// --- BehaviorJump ---
void BehaviorJump::Initialize(Player* player) {
    auto state = player->GetState();
    if (state) {
        auto jumpAction = state->GetJumpAction();
        if (jumpAction) {
            jumpAction->Execute(player);
        }
    }
}

void BehaviorJump::Update(Player* player) {
    if (player->IsGround()) {
        if (auto state = player->GetState()) {
            // ★ Factory 経由で Root に戻る
            if (auto factory = state->GetFactory()) {
                state->ChangeBehavior(player, factory->CreateBehavior(BehaviorType::Root));
            }
        }
    }
    player->UpdateGravity();
}

void BehaviorJump::Finalize(Player* player) {}

void BehaviorJump::HandleInput(Player* player, ICommand* command) {
    auto state = player->GetState();
    if (!state) return;

    auto moveAction = state->GetMoveAction();

    if (auto moveCmd = dynamic_cast<MoveCommand*>(command)) {
        if (moveAction) {
            static_cast<NormalMoveAction*>(moveAction)->SetSpeed(moveCmd->GetSpeed());
            moveAction->Execute(player);
        }
    }

    // ★ Factory を取得して生成
    auto factory = state->GetFactory();
    if (!factory) return;

    if (dynamic_cast<AttackCommand*>(command)) {
        state->ChangeBehavior(player, factory->CreateBehavior(BehaviorType::Attack));
    }
    if (dynamic_cast<PreShootCommand*>(command)) {
        if (dynamic_cast<IStateRideOn*>(state)) {
            state->ChangeBehavior(player, factory->CreateBehavior(BehaviorType::Aim));
        }
    }
}

// --- BehaviorAim ---
void BehaviorAim::Initialize(Player* player) {
    aimX_ = (float)player->GetMoveDirection();
    aimY_ = 0.0f;
}

void BehaviorAim::Update(Player* player) {}
void BehaviorAim::Finalize(Player* player) {}

void BehaviorAim::HandleInput(Player* player, ICommand* command) {
    if (auto aimCmd = dynamic_cast<AimCommand*>(command)) {
        aimX_ = aimCmd->GetX();
        aimY_ = aimCmd->GetY();
    }

    if (dynamic_cast<ShootCommand*>(command)) {
        auto state = dynamic_cast<IStateRideOn*>(player->GetState());
        if (state) {
            auto shootAction = state->GetShootAction();
            if (shootAction) {
                static_cast<ShootRobotAction*>(shootAction)->SetAimVector(aimX_, aimY_);
                shootAction->Execute(player);
            }
        }
    }
}
// --- BehaviorBound ---
void BehaviorBound::Initialize(Player* player) {
    // 状態開始と同時に最初の跳躍を実行
    if (auto state = player->GetState()) {
        if (auto jumpAction = state->GetJumpAction()) {
            jumpAction->Execute(player);
        }
    }
}

void BehaviorBound::Update(Player* player) {
    // 地面に就いた瞬間に自動で再度ジャンプする（常に跳ね続ける）
    if (player->IsGround()) {
        if (auto state = player->GetState()) {
            if (auto jumpAction = state->GetJumpAction()) {
                jumpAction->Execute(player);
            }
        }
    }

    // 重力更新
    player->UpdateGravity();
}

void BehaviorBound::Finalize(Player* player) {}

void BehaviorBound::HandleInput(Player* player, ICommand* command) {
    auto state = player->GetState();
    if (!state) return;

    // 空中・着地問わず左右移動は受け付ける
    if (auto moveCmd = dynamic_cast<MoveCommand*>(command)) {
        if (auto moveAction = state->GetMoveAction()) {
            static_cast<NormalMoveAction*>(moveAction)->SetSpeed(moveCmd->GetSpeed());
            moveAction->Execute(player);
        }
    }
    // ★ Factory を取得して生成
    auto factory = state->GetFactory();
    if (!factory) return;

    if (dynamic_cast<AttackCommand*>(command)) {
        state->ChangeBehavior(player, factory->CreateBehavior(BehaviorType::Attack));
    }
    if (dynamic_cast<PreShootCommand*>(command)) {
        if (dynamic_cast<IStateRideOn*>(state)) {
            state->ChangeBehavior(player, factory->CreateBehavior(BehaviorType::Aim));
        }
    }
}