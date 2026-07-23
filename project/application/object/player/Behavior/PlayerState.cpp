#include "PlayerState.h"
#include "Player.h"

void IPlayerState::Initialize(Player* player) {
    if (factory_) {
        // Factory からそれぞれの Action を組み立て
        moveAction_ = factory_->CreateMoveAction();
        jumpAction_ = factory_->CreateJumpAction();
        attackAction_ = factory_->CreateAttackAction();

        // 初期 Behavior を Factory から作成してセット
        ChangeBehavior(player, factory_->CreateBehavior(BehaviorType::Root));
    }
}

void IPlayerState::Update(Player* player) {
    if (currentBehavior_) currentBehavior_->Update(player);
}

void IPlayerState::Finalize(Player* player) {
    if (currentBehavior_) {
        currentBehavior_->Finalize(player);
        currentBehavior_.reset();
    }
}

void IPlayerState::HandleInput(Player* player, ICommand* command) {
    if (currentBehavior_) {
        currentBehavior_->HandleInput(player, command);
    }
}

void IPlayerState::ChangeBehavior(Player* player, std::unique_ptr<IPlayerBehavior> next) {
    if (!factory_) return;
    
    if (currentBehavior_) {
        currentBehavior_->Finalize(player);
    }
    currentBehavior_ = std::move(next);
    if (currentBehavior_) {
        currentBehavior_->Initialize(player);
    }
}

// --- IStateRideOn ---
void IStateRideOn::Initialize(Player* player) {
    // 基底クラスの Action / Behavior 生成を実行
    IPlayerState::Initialize(player);

    // RideOn 固有の ShootAction を Factory から取得
    if (factory_) {
        shootAction_ = factory_->CreateShootAction();
    }
}

void IStateRideOn::DoMove(Player* player) {
    if (moveAction_) moveAction_->Execute(player);
}

void IStateRideOn::DoShoot(Player* player) {
    if (shootAction_) shootAction_->Execute(player);
}