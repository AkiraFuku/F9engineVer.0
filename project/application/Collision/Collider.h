// Collider.h
#pragma once
#include "Vector3.h"
#include "GameObject.h"

class GameObject;


class Collider {
public:
    Collider() = default;
    ~Collider() = default;

    void initialize(GameObject* owner, float radius);
  

    // 持ち主（GameObject）から座標を取得
    Vector3 GetWorldPosition() const;

    float GetRadius() const { return radius_; }
    void SetRadius(float radius) { radius_ = radius; }

    CollisionCategory GetCategory() const { return category_; }
    void SetCategory(CollisionCategory category) { category_ = category; }

    GameObject* GetOwner() const { return owner_; }

    // 衝突検知時に呼び出す（処理はオーナーに委譲）
    void OnCollision(Collider* other);

private:
    GameObject* owner_ = nullptr;
    CollisionCategory category_ = CollisionCategory::Player;
    float radius_ = 1.0f;
};