#include "PlayerState.h"
#include "PlayerAction.h"
#include "Command.h"
#include "Player.h"
#include "PlayerBehavior.h"

void IPlayerState::Update(Player* player) {
    if (currentBehavior_) {
        currentBehavior_->Update(player);
    }
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
    if (currentBehavior_) {
        currentBehavior_->Finalize(player);
    }
    currentBehavior_ = std::move(next);
    if (currentBehavior_) {
        currentBehavior_->Initialize(player);
    }
}

// --- StateNormal ---
void StateNormal::Initialize(Player* player) {
    moveAction_ = std::make_unique<NormalMoveAction>(1.0f);
    jumpAction_ = std::make_unique<NormalJumpAction>();
    attackAction_ = std::make_unique<NormalAttackAction>();

    // 初期ビヘイビアを自前で生成・初期化
    ChangeBehavior(player, std::make_unique<BehaviorRoot>());
}

void StateNormal::Update(Player* player) {
    IPlayerState::Update(player);
}

void StateNormal::BehaviorUpdate(Player* player) {
    player->UpdateGravity();
}

void StateNormal::Finalize(Player* player) {
    IPlayerState::Finalize(player);
}

void StateNormal::HandleInput(Player* player, ICommand* command) {
    IPlayerState::HandleInput(player, command);
}

// --- IStateRideOn ---
IStateRideOn::IStateRideOn(std::unique_ptr<IPlayerAction> move, std::unique_ptr<IPlayerAction> jump,
                           std::unique_ptr<IPlayerAction> attack, std::unique_ptr<IPlayerAction> shoot)
    : moveAction_(std::move(move)), jumpAction_(std::move(jump)), attackAction_(std::move(attack)), shootAction_(std::move(shoot)) {}

void IStateRideOn::Update(Player* player) {
    IPlayerState::Update(player);
}

void IStateRideOn::Finalize(Player* player) {
    IPlayerState::Finalize(player);
}

void IStateRideOn::HandleInput(Player* player, ICommand* command) {
    IPlayerState::HandleInput(player, command);
}

void IStateRideOn::DoMove(Player* player) {
    if (moveAction_) moveAction_->Execute(player);
}

void IStateRideOn::DoShoot(Player* player) {
    if (shootAction_) shootAction_->Execute(player);
}

// --- StateRideOnTest ---
StateRideOnTest::StateRideOnTest()
    : IStateRideOn(
        std::make_unique<NormalMoveAction>(0.15f),
        std::make_unique<NormalJumpAction>(),
        std::make_unique<NormalAttackAction>(),
        std::make_unique<ShootRobotAction>()
    ) {}

void StateRideOnTest::Initialize(Player* player) {
    ChangeBehavior(player, std::make_unique<BehaviorRoot>());
}

void StateRideOnTest::BehaviorUpdate(Player* player) {
    player->UpdateGravity();
}

void StateRideOnTest::HandleInput(Player* player, ICommand* command) {
    IPlayerState::HandleInput(player, command);
}

// --- StateDead ---
void StateDead::Initialize(Player* player) {}
void StateDead::Update(Player* player) {}
void StateDead::BehaviorUpdate(Player* player) {}
void StateDead::Finalize(Player* player) { IPlayerState::Finalize(player); }
void StateDead::HandleInput(Player* player, ICommand* command) {}