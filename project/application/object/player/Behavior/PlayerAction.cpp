#include "PlayerAction.h"
#include "Player.h"
void NormalMoveAction::Execute(Player* player){
    // Player.cpp にあった移動ロジックをここに移譲
    player->Move(speed_);
}

void NormalJumpAction::Execute(Player* player){
    player->Jump();

}
void NormalAttackAction::Execute(Player* player) {
// 1. 入力から方向を決定する (InputHandler経由、あるいは現在の移動スティック値など)
    // ここでは例として「スティックが倒されていればその方向、なければレール方向」とします
    
    int attackDir;
    // 仮：スティック入力を取得できる仕組みがある場合
    // Vector2 input = player->GetInputHandler()->GetMoveStick(); 
    // if (input.Length() > 0.1f) { 
    //     attackDir = {input.x, 0, input.y}; 
    // } else { 
    //     attackDir = player->(); 
    // }

    attackDir = player->GetMoveDirection(); // 一旦現状維持

    // 2. プレイヤーのモデルをその方向に向ける
    
    // 3. その方向に突進（移動）
    // レール上移動の場合、attackDir と レール方向の「内積」をとることで、
    // 前に進むか後ろに戻るかを決定できます
    Vector3 railForward = player->GetDirection();
    float moveAmount = float(attackDir) * dashSpeed_;
    
    player->Move(moveAmount);
}