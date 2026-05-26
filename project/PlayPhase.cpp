#include "PlayPhase.h"
#include "GameScene.h"
#include "Player.h"
#include "CollisionManager.h"
#include "GoalObject.h"
#include "CameraController.h"
#include "RailPath.h"
#include "Projectile.h"
#include "Enemy.h"

void PlayPhase::Initialize(Scene* scene)
{
}

void PlayPhase::Update(Scene* scene)
{
    GameScene* gameScene = static_cast<GameScene*>(scene);

    gameScene->GetGoal()->Update();
    gameScene->GetCamera()->Update();
    gameScene->GetStageRall()->Update();
    gameScene->GetPlayer()->Update();

  // --- 修正箇所: auto& (参照) で受け取る ---
    const auto& projectiles = gameScene->GetProjectile();
    for (auto& projectile : projectiles) {
        if (projectile) {
            projectile->Update();
        }
    }
    // ※注意：参照で受け取っている場合、ここでの erase は不要（あるいは不可）です。
    // 消去(remove_if)は、元の所有者である GameScene 側で行うべき処理です。

    // --- 修正箇所: auto& (参照) で受け取る ---
    const auto& enemies = gameScene->GetEnemies();
    for (auto& enemy : enemies) {
        enemy->Update();
    }
    // ※同様にここでの erase も削除します。

    // --- 衝突判定の実行 ---
    CollisionManager* colManager = CollisionManager::GetInstance();


    colManager->CheckAllCollisions();

}

void PlayPhase::Draw(Scene* scene)
{
}

void PlayPhase::Finalize(Scene* scene)
{
}
