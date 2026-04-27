#include "PlayerState.h"
#include "PlayerAction.h"
#include "Command.h"
#include <memory>
#include "Player.h"
#include "PlayerBehavior.h"

StateNormal::StateNormal() = default;
StateNormal::~StateNormal() = default;

//通常状態
void StateNormal::Initialize(Player* player)
{
    if (player->GetBehavior()) {
        player->GetBehavior()->Initialize(player);
    }
    moveAction_ = std::make_unique<NormalMoveAction>(1.0f); // デフォルト速度
    jumpAction_ = std::make_unique<NormalJumpAction>();
    attackAction_ = std::make_unique<NormalAttackAction>(); // ここで生成
}

void StateNormal::Update(Player* player)
{
     if (player->GetBehavior()) {
         player->GetBehavior()->Update(player);
     }
   //  player->UpdateGravity();
}
void StateNormal::BehaviorUpdate(Player* player)
{
   player->UpdateGravity();
}

void StateNormal::Finalize(Player* player)
{
    if (player->GetBehavior()) {
        player->GetBehavior()->Finalize(player);
    }
}

void StateNormal::HandleInput(Player* player, ICommand* command)
{
    if (player->GetBehavior()) {
        player->GetBehavior()->HandleInput(player, command);
    }
}



IStateRideOn::IStateRideOn(std::unique_ptr<IPlayerAction> move, std::unique_ptr<IPlayerAction> attack)
    : moveAction_(std::move(move)), attackAction_(std::move(attack)) {
}

void IStateRideOn::Update(Player* player)
{
    if (player->GetBehavior()) {
        player->GetBehavior()->Update(player);
    }
}
void IStateRideOn::Finalize(Player* player) {
    // 1. ロボットオブジェクトを弾として生成（またはEnemyを再利用して飛ばす）
    // LaunchRobot(player->GetPosition(), shootDirection);

    // 2. プレイヤーに反動を与える
    // Vector3 recoil = shootDirection * -1.0f * kRecoilPower;
    // player->SetVelocity(recoil); 
}
void IStateRideOn::HandleInput(Player* player, ICommand* command)
{
    if (player->GetBehavior()) {
        player->GetBehavior()->HandleInput(player, command);
    }
}

void IStateRideOn::DoMove(Player* player)
{
    if (moveAction_) moveAction_->Execute(player);
}

void IStateRideOn::DoShoot(Player* player)
{
    if (shootAction_)shootAction_->Execute(player);
}
