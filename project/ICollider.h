#pragma once
#include "Vector3.h"
enum class CollisionCategory {
    Player,
    Enemy,
    PlayerProjectile,
    EnemyProjectile,
    Goal,
    Terrain,
    // 必要に応じて他のカテゴリも追加

};
class ICollider
{
    public:
    virtual ~ICollider() = default;

    // 衝突時に呼ばれる通知関数
    virtual void OnCollision(ICollider* other) = 0;

    // 判定に必要な情報のゲッター
    virtual Vector3 GetWorldPosition() const = 0;
    virtual float GetRadius() const = 0;
    /// <summary>
    /// ぶつかった相手のカテゴリを識別するための関数
    /// </summary>
    /// <returns></returns>
    virtual CollisionCategory GetCategory() const = 0;
};

