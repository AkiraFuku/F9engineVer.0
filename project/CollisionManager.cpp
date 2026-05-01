#include "CollisionManager.h"
#include "Player.h"
#include "Enemy.h"
#include <cmath>
#include "Projectile.h"
#include "MathFunction.h"
std::unique_ptr<CollisionManager> CollisionManager::instance = nullptr;

CollisionManager* CollisionManager::GetInstance() {
    if (instance == nullptr) {
        struct Helper : public CollisionManager {
            Helper() : CollisionManager() {}
        };
        instance = std::make_unique<Helper>();
    }
    return instance.get();
}

void CollisionManager::Finalize() {
    instance.reset();
}

/*void CollisionManager::CheckAllCollisions() {
    if (!player_) return;

    for (Enemy* enemy : enemies_) {
        CheckPlayerEnemyCollision(player_, enemy);
    }
}*/

void CollisionManager::CheckPlayerEnemyCollision(Player* p, Enemy* e) {
    Vector3 posP = p->GetTransform().translate;
    Vector3 posE = e->GetTransform().translate;

   /* float dx = posP.x - posE.x;
    float dy = posP.y - posE.y;
    float dz = posP.z - posE.z;*/
float distanceSq = Length(Subtract(posP, posE));
    // 半径を仮に 1.0f ずつとして、距離の2乗で判定 (1.0 + 1.0)^2 = 4.0
    float radiusSum = 2.0f; 
    if (distanceSq <= radiusSum * radiusSum) {
        // --- ここでプレイヤーとエネミーに通知する ---
       
        e->OnCollision(p);
        p->OnCollision(e);
    }
}
void CollisionManager::CheckAllCollisions() {
    // 既存のプレイヤー vs 敵の判定
    if (player_) {
        for (Enemy* enemy : enemies_) {
            CheckPlayerEnemyCollision(player_, enemy);
        }
    }

    // 弾に関連する判定
    for (Projectile* projectile : projectiles_) {
        if (projectile->IsDead()) continue;

        // プレイヤーが撃った弾なら、敵との判定を行う
        if (projectile->GetOwner() == Projectile::ProjectileOwner::Player) {
            for (Enemy* enemy : enemies_) {
                CheckProjectileEnemyCollision(projectile, enemy);
            }
        }
        // 敵が撃った弾なら、プレイヤーとの判定を行う
        else if (projectile->GetOwner() == Projectile::ProjectileOwner::Enemy) {
            if (player_) {
                CheckProjectilePlayerCollision(projectile, player_);
            }
        }
    }
}

void CollisionManager::CheckProjectileEnemyCollision(Projectile* p, Enemy* e) {
    // 球体同士の判定ロジック
    Vector3 posP = p->GetPosition();
    Vector3 posE = e->GetTransform().translate;

    float distanceSq = Length(Subtract(posP, posE));
    float radiusSum = p->GetRadius() + 1.0f; // 敵の半径を仮に1.0とする

    if (distanceSq <= radiusSum * radiusSum) {
        p->OnCollision(); // 弾の消滅処理など
        // 敵の被弾処理（Enemy側に弾用のOnCollisionが必要な場合は作成してください）
         e->OnCollision(); 
    }
}
void CollisionManager::CheckProjectilePlayerCollision(Projectile* p, Player* player) {
    // 球体同士の判定ロジック
    Vector3 posP = p->GetPosition();
    Vector3 posPlayer = player->GetTransform().translate;

    float distanceSq = Length(Subtract(posP, posPlayer));
    float radiusSum = p->GetRadius() + 1.0f; // 敵の半径を仮に1.0とする

    if (distanceSq <= radiusSum * radiusSum) {
        p->OnCollision(); // 弾の消滅処理など
        // 敵の被弾処理（Enemy側に弾用のOnCollisionが必要な場合は作成してください）
       //  e->OCollision(); 
    }
}