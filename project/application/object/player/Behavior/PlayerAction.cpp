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

void ShootRobotAction::Execute(Player* player)
{
    // プレイヤーの情報を取得
    const RailPath* path = player->GetRailPath();
    float currentT = player->GetRailProgress(); // Player側に GetProgress() が必要
    int moveDir = player->GetMoveDirection();   // 1(順) or -1(逆)

    // GameSceneへの弾の追加
    Scene* scene = player->GetScene();
    if (scene) {
        float bulletSpeed = 0.5f; // プレイヤーより速い速度
        Vector3 bulletPosition = { currentT, 0.0f, 0.0f };

        // ダウンキャスト（GameSceneに変換）
        GameScene* gameScene = dynamic_cast<GameScene*>(scene);
        if (gameScene) {
            gameScene->AddProjectile(bulletPosition, (float)moveDir * bulletSpeed);
        }
    }
}
