#include "PlayerState.h"
#include "PlayerAction.h"
#include "Command.h"
#include <memory>

StateNormal::StateNormal()=default;
StateNormal::~StateNormal()=default;

//通常状態
void StateNormal::Initialize(Player* player)
{
}

void StateNormal::Update(Player* player)
{
}

void StateNormal::Finalize(Player* player)
{
}

void StateNormal::HandleInput(Player* player, ICommand* command)
{
    // 移動コマンドが来たら
    if (auto moveCmd = dynamic_cast<MoveCommand*>(command)) {
        // 通常移動アクションを実行 (速度はコマンドから取得)
        auto action = std::make_unique<NormalMoveAction>(moveCmd->GetSpeed());
        action->Execute(player);
    }
    // ジャンプコマンドが来たら
    else if (dynamic_cast<JumpCommand*>(command)) {
        auto action = std::make_unique<NormalJumpAction>();
        action->Execute(player);
    }
}


//ロボットに搭乗している状態
StateRideOn::StateRideOn(std::unique_ptr<IPlayerAction> moveAction,
                         std::unique_ptr<IPlayerAction> attackAction)
    : moveAction_(std::move(moveAction)), 
      attackAction_(std::move(attackAction)) 
{
   jumpAction_=nullptr;
}
StateRideOn::~StateRideOn() = default;
void StateRideOn::Initialize(Player* player)
{
}

void StateRideOn::Update(Player* player)
{
}

void StateRideOn::Finalize(Player* player)
{
}

void StateRideOn::HandleInput(Player* player, ICommand* command)
{
    // RTTIやタイプIDを使って、送られてきたコマンドの種類を判別
    if (auto moveCmd = dynamic_cast<MoveCommand*>(command)) {
        // ロボット専用の移動アクションを実行
        moveAction_->Execute(player); 
    }
    else if (auto attackCmd = dynamic_cast<AttackCommand*>(command)) {
        // ロボットの種類に応じた攻撃アクションを実行
        attackAction_->Execute(player);
    }
}

