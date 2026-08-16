#include "PlayPhase.h"
#include "GameScene.h"
#include "Player.h"
#include "CollisionManager.h"
#include "GoalObject.h"
#include "CameraController.h"
#include "RailPath.h"
#include "Projectile.h"
#include "Enemy.h"
#include "MiniBoss.h"

void PlayPhase::Initialize(Scene* scene)
{}

void PlayPhase::Update(Scene* scene)
{
    GameScene* gameScene = static_cast<GameScene*>(scene);

    gameScene->GetGoal()->Update();
    gameScene->GetCamera()->Update();
    gameScene->GetStageRaill()->Update();

   

    // 2. ヒットストップ中はキャラクターや物理演算、衝突判定の更新をスキップする
    if (gameScene->IsHitStopActive()) {
        return; // ここから下の更新・衝突判定は一切行わない
    }

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
    std::vector<Collider*> colliders;

    const auto& player = gameScene->GetPlayer();
    const auto& goal_ = gameScene->GetGoal();


    if (player && player->GetCollider()) {
        colliders.push_back(player->GetCollider());
    }

    for (auto& enemy : enemies) {
        if (enemy && !enemy->IsDead() && enemy->GetCollider()) {
            //ミニボスのパーツも含めて衝突判定を行う
            if (auto miniBoss = dynamic_cast<MiniBoss*>(enemy.get())) {
                for (const auto& collider : miniBoss->GetColliders()) {
                    colliders.push_back(collider);
                }
                continue; // ミニボスのパーツを追加したので、次の敵へ
            }

            colliders.push_back(enemy->GetCollider());
        }
    }

    for (auto& projectile : projectiles) {
        if (projectile && !projectile->IsDead() && projectile->GetCollider()) {
            colliders.push_back(projectile->GetCollider());
        }
    }

    if (goal_ && goal_->GetCollider()) {
        colliders.push_back(goal_->GetCollider());
    }
    colManager->CheckAllCollisions(colliders);


}

void PlayPhase::Draw(Scene* scene)
{

}

void PlayPhase::Finalize(Scene* scene)
{}
