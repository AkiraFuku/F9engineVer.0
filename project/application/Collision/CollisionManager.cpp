#include "CollisionManager.h"
#include "Player.h"
#include "Enemy.h"
#include <cmath>
#include "Projectile.h"
#include "MathFunction.h"
#include "GoalObject.h"
#include "GameScene.h"
std::unique_ptr<CollisionManager> CollisionManager::instance = nullptr;

CollisionManager* CollisionManager::GetInstance() {
    if (instance == nullptr) {
        struct Helper : public CollisionManager {
            Helper() : CollisionManager() {
            }
        };
        instance = std::make_unique<Helper>();
    }
    return instance.get();
}



void CollisionManager::CheckCollision(ICollider* a, ICollider* b) {
    Vector3 posA = a->GetWorldPosition();
    Vector3 posB = b->GetWorldPosition();

    float distanceSq = Length(Subtract(posA, posB));
    float radiusSum = a->GetRadius() + b->GetRadius();

    if (distanceSq <= radiusSum) {
        // お互いに「相手」を渡して通知する
        a->OnCollision(b);
        b->OnCollision(a);
    }
}

void CollisionManager::SetScene(GameScene* scene)
{
    scene_ = scene;
}



void CollisionManager::Finalize() {
    instance.reset();
}
void CollisionManager::CheckAllCollisions() {
    Clear();
    if (scene_)
    {
        // 敵のリストを参照で取得し、生のポインタを登録用vectorに入れる
        const auto& sceneEnemies = scene_->GetEnemies();
        for (auto& enemy : sceneEnemies) {
            enemies_.push_back(enemy.get());
        }

        // 弾のリストも同様
        const auto& sceneProjectiles = scene_->GetProjectile();
        for (auto& projectile : sceneProjectiles) {
            projectiles_.push_back(projectile.get());
        }

        // プレイヤーとゴールも最新の状態を取得
        player_ = const_cast<Player*>(scene_->GetPlayer());
        goal_ = const_cast<GoalObject*>(scene_->GetGoal());

    }


    // 既存のプレイヤー vs 敵の判定
    if (player_) {
        if (!enemies_.empty())
        {
            for (Enemy* enemy : enemies_) {
                CheckCollision( enemy,player_);
            }
        }

    }
    if (!projectiles_.empty())
    {
        // 弾に関連する判定
        for (Projectile* projectile : projectiles_) {
            if (projectile->IsDead()) continue;

            // プレイヤーが撃った弾なら、敵との判定を行う
            if (projectile->GetOwner() == Projectile::ProjectileOwner::Player) {
                if (!enemies_.empty())
                {
                    for (Enemy* enemy : enemies_) {
                        CheckCollision(projectile, enemy);
                    }
                }
            }
            // 敵が撃った弾なら、プレイヤーとの判定を行う
            else if (projectile->GetOwner() == Projectile::ProjectileOwner::Enemy) {
                if (player_) {
                    CheckCollision(projectile, player_);
                }
            }
        }
    }

    if (player_ && goal_)
    {
        CheckCollision(player_, goal_);
    }

}

//void CollisionManager::CheckProjectileEnemyCollision(Projectile* p, Enemy* e) {
//    // 球体同士の判定ロジック
//    Vector3 posP = p->GetWorldPosition();
//    Vector3 posE = e->GetTransform().translate;
//
//    float distanceSq = Length(Subtract(posP, posE));
//    float radiusSum = p->GetRadius() + 1.0f; // 敵の半径を仮に1.0とする
//
//    if (distanceSq <= radiusSum) {
//        p->OnCollision(); // 弾の消滅処理など
//        // 敵の被弾処理（Enemy側に弾用のOnCollisionが必要な場合は作成してください）
//        e->OnCollision();
//    }
//}
//void CollisionManager::CheckProjectilePlayerCollision(Projectile* p, Player* player) {
//    // 球体同士の判定ロジック
//    Vector3 posP = p->GetWorldPosition();
//    Vector3 posPlayer = player->GetTransform().translate;
//
//    float distanceSq = Length(Subtract(posP, posPlayer));
//    float radiusSum = p->GetRadius() + player->GetRadius(); // プレイヤーの半径を取得
//
//    if (distanceSq <= radiusSum) {
//        p->OnCollision(); // 弾の消滅処理など
//        // プレイヤーの被弾処理（Player側に弾用のOnCollisionが必要な場合は作成してください）
//       //  player->OnCollision(); 
//    }
//}