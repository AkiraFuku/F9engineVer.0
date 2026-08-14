#include "CollisionManager.h"
#include "Collider.h"
#include "MathFunction.h" // SubtractやLengthSqなどの数学関数用
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
void CollisionManager::CheckAllCollisions(const std::vector<Collider*>& colliders) {
    size_t count = colliders.size();
    if (count < 2) return;

    for (size_t i = 0; i < count; ++i) {
        Collider* colA = colliders[i];
        if (!colA) continue;

        for (size_t j = i + 1; j < count; ++j) {
            Collider* colB = colliders[j];
            if (!colB) continue;

            // 1. カテゴリチェックで不要なペアを即座にスキップ（最優先で高速化）
            if (!ShouldCheckCollision(colA->GetCategory(), colB->GetCategory())) {
                continue;
            }

            // 2. 距離によるブロードフェーズ除外
            Vector3 posA = colA->GetWorldPosition();
            Vector3 posB = colB->GetWorldPosition();
            Vector3 diff = Subtract(posA, posB);
            float distSq = (diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z);

            if (distSq > kBroadPhaseMaxDistanceSq) {
                continue;
            }

            // 3. 優先度の取得と並び替え
            int priorityA = GetCategoryPriority(colA->GetCategory());
            int priorityB = GetCategoryPriority(colB->GetCategory());

            Collider* first = colA;
            Collider* second = colB;

            if (priorityB > priorityA) {
                first = colB;
                second = colA;
            }

            // 4. 精密判定とイベント呼び出し
            CheckCollision(first, second);
        }
    }
}

int CollisionManager::GetCategoryPriority(CollisionCategory category)
{
    switch (category) {
    case CollisionCategory::Collectible:      return 120; // 収集アイテム最優先
    case CollisionCategory::CollisionObject:  return 100; // めり込めないオブジェクト最優先
    case CollisionCategory::Attackable:       return 100; // 攻撃可能オブジェクト最優先
    case CollisionCategory::InvincibleEnemy:  return 100; // 攻撃（無敵）不可エネミー最優先
    case CollisionCategory::Enemy:            return 100; // エネミー最優先
    case CollisionCategory::Player:           return 80;  // プレイヤー
    case CollisionCategory::PlayerProjectile: return 50;  // プレイヤーの弾
    case CollisionCategory::EnemyProjectile:  return 50;  // 敵の弾
    case CollisionCategory::Goal:             return 10;  // ゴール
    default:                                  return 0;
    }
}

void CollisionManager::CheckCollision(Collider* a, Collider* b) {
    Vector3 posA = a->GetWorldPosition();
    Vector3 posB = b->GetWorldPosition();

    if ((!a->IsCollide()) || (!b->IsCollide()))
    {
        return; // どちらかが衝突不可なら判定しない

    }

    Vector3 diff = Subtract(posA, posB);
    float distanceSq = (diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z);

    float radiusSum = a->GetRadius() + b->GetRadius();
    float radiusSumSq = radiusSum * radiusSum; // 半径和も2乗で比較

    if (distanceSq <= radiusSumSq) {
        // お互いに「相手」を渡して通知する
        a->OnCollision(b);
        b->OnCollision(a);

    }
}

bool CollisionManager::ShouldCheckCollision(CollisionCategory catA, CollisionCategory catB) const {
    // 同じカテゴリ同士（敵同士、弾同士など）は判定しない
    if (catA == catB) return false;

    // プレイヤー × 敵[cite: 13, 16]
    if ((catA == CollisionCategory::Player && catB == CollisionCategory::Enemy)||(catA == CollisionCategory::Enemy && catB == CollisionCategory::Player)) {
        return true;
    }
    // プレイヤー × 収集アイテム[cite: 13, 16]
    if ((catA == CollisionCategory::Player && catB == CollisionCategory::Collectible) ||
        (catA == CollisionCategory::Collectible && catB == CollisionCategory::Player)) {
        return true;
    }
    // プレイヤー × めり込めないオブジェクト (CollisionObject)
    if ((catA == CollisionCategory::Player && catB == CollisionCategory::CollisionObject) ||
        (catA == CollisionCategory::CollisionObject && catB == CollisionCategory::Player)) {
        return true;
    }

    // プレイヤー × 攻撃不可エネミー (InvincibleEnemy) 
    if ((catA == CollisionCategory::Player && catB == CollisionCategory::InvincibleEnemy) ||
        (catA == CollisionCategory::InvincibleEnemy && catB == CollisionCategory::Player)) {
        return true;
    }

    // プレイヤー × 攻撃可能オブジェクト/弱点 (Attackable)
    if ((catA == CollisionCategory::Player && catB == CollisionCategory::Attackable) ||
        (catA == CollisionCategory::Attackable && catB == CollisionCategory::Player)) {
        return true;
    }

    // プレイヤーの弾 × 攻撃可能オブジェクト/弱点 (Attackable)
    if ((catA == CollisionCategory::PlayerProjectile && catB == CollisionCategory::Attackable) ||
        (catA == CollisionCategory::Attackable && catB == CollisionCategory::PlayerProjectile)) {
        return true;
    }
    // プレイヤー × 敵の弾[cite: 13, 16]
    if ((catA == CollisionCategory::Player && catB == CollisionCategory::EnemyProjectile) ||
        (catA == CollisionCategory::EnemyProjectile && catB == CollisionCategory::Player)) {
        return true;
    }

    // 敵 × プレイヤーの弾[cite: 13, 16]
    if ((catA == CollisionCategory::Enemy && catB == CollisionCategory::PlayerProjectile) ||
        (catA == CollisionCategory::PlayerProjectile && catB == CollisionCategory::Enemy)) {
        return true;
    }

    // プレイヤー × ゴール[cite: 13, 16]
    if ((catA == CollisionCategory::Player && catB == CollisionCategory::Goal) ||
        (catA == CollisionCategory::Goal && catB == CollisionCategory::Player)) {
        return true;
    }

    return false; // それ以外は無視
}