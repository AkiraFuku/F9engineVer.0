#include "PlayerAction.h"
#include "Player.h"
void NormalMoveAction::Execute(Player* player){
    // Player.cpp にあった移動ロジックをここに移譲
    player->Move(speed_);
}

void NormalJumpAction::Execute(Player* player){
    player->Jump();

}
