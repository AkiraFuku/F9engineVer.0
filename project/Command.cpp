#include "Command.h"
#include "Player.h"
void MoveCommand::Execute(Player& player)
{
    player.Move(speed_);
}

void JumpCommand::Execute(Player& player)
{
    player.Jump();
}

void AttackCommand::Execute(Player& player)
{
}
