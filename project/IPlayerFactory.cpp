#include "IPlayerFactory.h"
#include "PlayerState.h"
#include "PlayerAction.h"
#include "PlayerBehavior.h"

// --- NormalPlayerFactory ---
std::unique_ptr<IPlayerState> NormalPlayerFactory::CreateState() {
    return std::make_unique<StateNormal>(shared_from_this());
}
std::unique_ptr<IPlayerAction> NormalPlayerFactory::CreateMoveAction() {
    return std::make_unique<NormalMoveAction>(1.0f);
}
std::unique_ptr<IPlayerAction> NormalPlayerFactory::CreateJumpAction() {
    return std::make_unique<NormalJumpAction>();
}
std::unique_ptr<IPlayerAction> NormalPlayerFactory::CreateAttackAction() {
    return std::make_unique<NormalAttackAction>();
}
std::unique_ptr<IPlayerBehavior> NormalPlayerFactory::CreateBehavior(BehaviorType type) {
    switch (type) {
    case BehaviorType::Jump:   return std::make_unique<BehaviorJump>();
    case BehaviorType::Attack: return std::make_unique<BehaviorAttack>();
    case BehaviorType::Root:
    default:                   return std::make_unique<BehaviorRoot>();
    }
}

// --- RideOnPlayerFactory ---
std::unique_ptr<IPlayerState> RideOnPlayerFactory::CreateState() {
    return std::make_unique<StateRideOnTest>(shared_from_this());
}
std::unique_ptr<IPlayerAction> RideOnPlayerFactory::CreateMoveAction() {
    return std::make_unique<NormalMoveAction>(0.15f);
}
std::unique_ptr<IPlayerAction> RideOnPlayerFactory::CreateJumpAction() {
    return std::make_unique<NormalJumpAction>();
}
std::unique_ptr<IPlayerAction> RideOnPlayerFactory::CreateAttackAction() {
    return std::make_unique<NormalAttackAction>();
}
std::unique_ptr<IPlayerAction> RideOnPlayerFactory::CreateShootAction() {
    return std::make_unique<ShootRobotAction>();
}
std::unique_ptr<IPlayerBehavior> RideOnPlayerFactory::CreateBehavior(BehaviorType type) {
    switch (type) {
    case BehaviorType::Jump:   return std::make_unique<BehaviorJump>();
    case BehaviorType::Attack: return std::make_unique<BehaviorAttack>();
    case BehaviorType::Aim:    return std::make_unique<BehaviorAim>();
    case BehaviorType::Root:
    default:                   return std::make_unique<BehaviorRoot>();
    }
}

std::unique_ptr<IPlayerState> PlayerStateFactory::CreateState(PlayerFormType type)
{

    std::shared_ptr<IPlayerFactory> factory = nullptr;

    switch (type) {
    case PlayerFormType::Normal:
        factory = std::make_shared<NormalPlayerFactory>();
        break;
    case PlayerFormType::RideOnTest:
        factory = std::make_shared<RideOnPlayerFactory>();
        break;
    }

    if (factory) {
        return factory->CreateState();
    }
    return nullptr;

}

std::unique_ptr<IPlayerAction> IPlayerFactory::CreateShootAction()
{
    return nullptr;
}
