// BossPart.h (または MiniBoss.h)
#pragma once
#include "GameObject.h"
#include "Collider.h"
#include "Object3d.h"

class MiniBoss; // 前方宣言
enum class BossPartType {
    Body,   // めり込めない胴体
    Weak,   // 弱点（攻撃可能）
    Armor   // 攻撃不可（装甲）めり込まない
};

// パーツの基底クラス（GameObjectを継承）
class BossPart : public GameObject {
public:
    BossPart(MiniBoss* owner, BossPartType type, float radius, CollisionCategory category, const Vector3& localPos)
        : ownerBoss_(owner), type_(type), localPosition_(localPos) 
    {
        collider_ = std::make_unique<Collider>();
        collider_->initialize(this, radius); // 自身の Collider に this を渡す
        collider_->SetCategory(category);
    }

    virtual ~BossPart() = default;

    // GameObjectの純粋仮想関数の実装
    Vector3 GetWorldPosition() const override ;

    CollisionCategory GetCategory() const override {
        return collider_ ? collider_->GetCategory() : CollisionCategory::Enemy;
    }

    // パーツ独自の衝突処理（派生クラスでオーバーライド）
    void OnCollision(GameObject* other) override = 0;

    void SetObject(std::unique_ptr<Object3d> obj) { object_ = std::move(obj); }
    Object3d* GetObject3D() { return object_.get(); }
    const Vector3& GetLocalPosition() const { return localPosition_; }

protected:
    MiniBoss* ownerBoss_ = nullptr;
    BossPartType type_;
    std::unique_ptr<Object3d> object_;
    Vector3 localPosition_;
};