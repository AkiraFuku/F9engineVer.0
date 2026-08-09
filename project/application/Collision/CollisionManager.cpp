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

    // 2重ループですべてのペアを網羅（重複なし）
    for (size_t i = 0; i < count; ++i) {
        Collider* colA = colliders[i];
        if (!colA) continue;

        for (size_t j = i + 1; j < count; ++j) {
            Collider* colB = colliders[j];
            if (!colB) continue;

            // 2. 距離による早期除外（ブロードフェーズ）
            Vector3 posA = colA->GetWorldPosition();
            Vector3 posB = colB->GetWorldPosition();
            Vector3 diff = Subtract(posA, posB);

            // 距離の2乗を計算（平方根の計算コストを回避）
            float distSq = (diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z);

            // 設定した最大距離より離れていればスキップ
            if (distSq > kBroadPhaseMaxDistanceSq) {
                continue;
            }

            // 1. 持ち主（カテゴリ）の組み合わせチェック（不要なペアは即スキップ）
            if (!ShouldCheckCollision(colA->GetCategory(), colB->GetCategory())) {
                continue;
            }
            // 優先度の取得
            int priorityA = GetCategoryPriority(colA->GetCategory());
            int priorityB = GetCategoryPriority(colB->GetCategory());

            Collider* first = colA;
            Collider* second = colB;

            // Bの方が優先度が高い場合は、Bを先に（第1引数に）する
            if (priorityB > priorityA) {
                first = colB;
                second = colA;
            }

            // 優先度が高い方の OnCollision が先に呼ばれる
            CheckCollision(first, second);

        }
    }
}

int CollisionManager::GetCategoryPriority(CollisionCategory category)
{
    switch (category) {
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
    if (catA == CollisionCategory::Player && catB == CollisionCategory::Enemy) {
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