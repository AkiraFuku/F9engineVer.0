#pragma once
#include "Vector3.h"
#include "DrawFunction.h"
#include <memory>
enum class CollisionCategory {
    Player=0,// プレイヤー
    Enemy=10,// 敵
    PlayerProjectile=20,// プレイヤーの弾
    EnemyProjectile=30,// 敵の弾
    Goal=40,// ゴール
    Collectible=50,// 収集アイテム
    CollisionObject=60, // めり込めないオブジェクト
    Attackable=70, // 攻撃可能オブジェクト
    InvincibleEnemy=80, // 攻撃（無敵）不可エネミー
    // 必要に応じて他のカテゴリも追加

};
class Collider;
class GameObject
{
public:
    virtual ~GameObject() = default;

    // 衝突時に呼ばれる通知関数
    virtual void OnCollision(GameObject* other) = 0;

    // 判定に必要な情報のゲッター
    virtual Vector3 GetWorldPosition() const = 0;

    // Colliderのゲッター
    virtual Collider* GetCollider() {
        return collider_.get();
    }
    /// <summary>
    /// ぶつかった相手のカテゴリを識別するための関数
    /// </summary>
    /// <returns></returns>
    virtual CollisionCategory GetCategory() const = 0;

    /// <summary>
    /// レイキャスト（接地判定など）の更新処理
    /// デフォルトでは何もしないため、不要なオブジェクトは実装しなくてOK
    /// </summary>
    virtual void RayCastUpdate() {};

    //
    struct GroundRayPalamata
    {
        //レイキャスト当たり判定用の変数
        float groundY = 0.0f;
        float rayOffset = 1.0f;
        const float minY = -10.0f; // 地面の最低Y座標
        const float kRayOffset = 2.0f; // レイの始点を上に持ち上げるオフセット

    };

protected:

    Vector3 Position_ = {};

    Ray ray_;
    bool isRayHit_ = false;
    Vector3 rayHitPoint_ = {};
    float rayHitDistance_ = 0.0f;
    Triangle rayHitTriangle_ = {};
    RayTriangleCollisionResult result_ = RayTriangleCollisionResult::NoCollision;

    // 自身のコライダー（必要に応じて派生クラスで初期化）
    std::unique_ptr<Collider> collider_;

};

