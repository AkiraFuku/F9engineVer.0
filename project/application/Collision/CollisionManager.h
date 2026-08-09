#pragma once
#include <memory>
#include <vector>
#include "GameObject.h" // CollisionCategoryが定義されているヘッダー[cite: 16]

class Collider;



class CollisionManager {
public:
    static CollisionManager* GetInstance();
    void Finalize();

    /// <summary>
    /// シーン等から渡されたコライダーリスト全体の衝突チェックを実行
    /// </summary>
    /// <param name="colliders">判定対象となるすべてのコライダー</param>
    void CheckAllCollisions(const std::vector<Collider*>& colliders);

private:
    CollisionManager() = default;
    ~CollisionManager() = default;
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;


   
    static std::unique_ptr<CollisionManager> instance;
    friend struct std::default_delete<CollisionManager>;

    // 衝突を検知・通知する基本処理[cite: 13]
    void CheckCollision(Collider* a, Collider* b);

    // カテゴリの組み合わせで判定が必要かをチェック
    bool ShouldCheckCollision(CollisionCategory catA, CollisionCategory catB) const;

    // 離れすぎているオブジェクト（例えば50.0f以上離れている等）を弾く距離閾値（二乗値）
    // 例: 50ユニット以上離れていたらスキップする場合は 50 * 50 = 2500.0f
    const float kBroadPhaseMaxDistanceSq = 2500.0f;

     int GetCategoryPriority(CollisionCategory category);

};