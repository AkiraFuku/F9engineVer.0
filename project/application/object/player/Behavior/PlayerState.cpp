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
}

void StateNormal::Update(Player* player)
{
    if (player->GetBehavior()) {
        player->GetBehavior()->Update(player);
    }
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

void IStateRideOn::HandleInput(Player* player, ICommand* command)
{
    if (player->GetBehavior()) {
        player->GetBehavior()->HandleInput(player, command);
    }
}

void IStateRideOn::DoMove(Player* player)
 { if(moveAction_) moveAction_->Execute(player); }

void IStateRideOn::DoShoot(Player * player)
{if(shootAction_)shootAction_->Execute(player);}
