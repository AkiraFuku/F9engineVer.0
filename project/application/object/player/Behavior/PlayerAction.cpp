#include "PlayerAction.h"
#include "Player.h"
void NormalMoveAction::Execute(Player* player){
    // Player.cpp にあった移動ロジックをここに移譲
    player->Move(speed_);
}

void NormalJumpAction::Execute(Player* player){
    player->Jump();

}
// PlayerAction.cpp
void NormalAttackAction::Execute(Player* player) {
    // 進行方向（1 or -1）を取得
    int attackDir = player->GetMoveDirection(); 

    // レールの進行方向ベクトルを取得
    Vector3 railForward = player->GetDirection();
    
    // 【修正ポイント】
    // 単純なMoveではなく、攻撃用の「瞬間的な移動量」を計算する
    // dashSpeed_ を現在の 0.5f から大幅に上げ（例: 2.0f）、
    // BehaviorAttack のタイマーに合わせて減衰させる処理を Behavior 側で行うのが理想です
    float moveAmount = float(attackDir) * dashSpeed_;
    
    player->Move(moveAmount);
}