#pragma once
#include "Object3d.h"
#include <memory>
#include "Vector3.h"
#include "Vector2.h"
#include "ICollider.h"
class RailPath;
class RailMover;
class Player;

class GoalObject : public ICollider {
public:
    GoalObject();
    ~GoalObject();

    void Initialize();
    void Update();
    void Draw();

    // レールと配置場所（進捗 0.0~1.0）を設定
    void SetRail(RailPath* rail);
    void SetRailPosition(const Vector2& position);

    void SetCamera(Camera* camera);

    // 衝突判定用
    void OnCollision(ICollider* other) override;
    bool IsCleared() const {
        return isCleared_;
    }

    Vector3 GetWorldPosition() const override {
        return object_->GetTranslate();
    }
    float GetRadius() const override {
        return radius_;
    }
    CollisionCategory GetCategory() const override {
        return CollisionCategory::Goal;
    }
private:
    std::unique_ptr<Object3d> object_;
    std::unique_ptr<RailMover> railMover_;

    float radius_ = 1.5f;   // 当たり判定の大きさ
    bool isCleared_ = false; // クリアフラグ
};