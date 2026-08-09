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
    void Update();
    void Draw() ;
    void SetOffset(const Vector3& offset) {
        Offset_ = offset;
    }
    void SetPosition(const Vector3& position) {
        position_ = position;
    }

    // 持ち主（GameObject）から座標を取得
    Vector3 GetWorldPosition() const;

    float GetRadius() const { return radius_; }
    void SetRadius(float radius) { radius_ = radius; }

    CollisionCategory GetCategory() const { return category_; }
    void SetCategory(CollisionCategory category) { category_ = category; }

    GameObject* GetOwner() const { return owner_; }


    // 衝突検知時に呼び出す（処理はオーナーに委譲）
    void OnCollision(Collider* other);

    bool IsCollide() const {
        return isCollide_;
    }
    void SetCollide(bool collide) {
        isCollide_ = collide;
    }


private:
    Vector3 position_ = {};
    Vector3 Offset_ = {};
    GameObject* owner_ = nullptr;
    CollisionCategory category_ = CollisionCategory::Player;
    float radius_ = 1.0f;
    bool isCollide_ = true; // 衝突可能か？
};