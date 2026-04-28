#include "PlayerAction.h"
#include "Player.h"
#include "Projectile.h"
#include "Scene.h"
#include "GameScene.h"
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

void ShootRobotAction::Execute(Player* player) {
    Scene* scene = player->GetScene();
    GameScene* gameScene = dynamic_cast<GameScene*>(scene);
    if (!gameScene) return;

    // 現在のプレイヤー位置をベースに設置位置を計算
    float currentT = player->GetRailProgress();
    float currentY = player->GetWorldY();
    
    // 1. 設置位置の決定（将来的に aimDir_ を使ってオフセット可能）
    Vector2 spawnPos = { currentT, currentY };

    // 2. 進行方向の決定
    // 今回は「弾の進む向きと設置方向を統一」するため aimDir_.x を使用
    float bulletSpeed = 0.5f * (aimDir_.x >= 0 ? 1.0f : -1.0f);

    gameScene->AddProjectile(spawnPos, bulletSpeed);
    player->ChangeBehavior(std::make_unique<BehaviorRoot>());
    player->ChangeState(std::make_unique<StateNormal>());
}
