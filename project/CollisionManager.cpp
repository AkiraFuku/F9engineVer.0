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
    // 座標取得 (GetTransform().translate など)
    Vector3 posP = p->GetTransform().translate;
    Vector3 posE = e->GetTransform().translate;

    // 距離の計算（球判定）
    float dx = posP.x - posE.x;
    float dy = posP.y - posE.y;
    float dz = posP.z - posE.z;
    float distanceSq = dx * dx + dy * dy + dz * dz;

    // 半径（仮に 1.0f とする。本来はクラスから取得すべき）
    float radiusSum = 1.0f + 1.0f; 

    if (distanceSq <= radiusSum * radiusSum) {
        // 衝突！
        p->OnCollision(e);
        e->OnCollision(p);
    }
}