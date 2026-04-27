#include "CollisionManager.h"
#include "CollisionManager.h"
#include "Player.h"
#include "Enemy.h"
#include <cmath>

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

void CollisionManager::CheckAllCollisions() {
    if (!player_) return;

    for (Enemy* enemy : enemies_) {
        CheckPlayerEnemyCollision(player_, enemy);
    }
}

void CollisionManager::CheckPlayerEnemyCollision(Player* p, Enemy* e) {
    Vector3 posP = p->GetTransform().translate;
    Vector3 posE = e->GetTransform().translate;

    float dx = posP.x - posE.x;
    float dy = posP.y - posE.y;
    float dz = posP.z - posE.z;
    float distanceSq = dx * dx + dy * dy + dz * dz;

    // 半径を仮に 1.0f ずつとして、距離の2乗で判定 (1.0 + 1.0)^2 = 4.0
    float radiusSum = 2.0f; 
    if (distanceSq <= radiusSum * radiusSum) {
        // --- ここでプレイヤーとエネミーに通知する ---
       
        e->OnCollision(p);
        p->OnCollision(e);
    }
}