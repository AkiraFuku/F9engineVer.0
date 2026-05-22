#pragma once
#include "Object3d.h"
#include <memory>
#include "Vector3.h"
#include "Vector2.h"
class RailPath;
class RailMover;
class Player;

class GoalObject {
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
    void OnCollision(Player* player);
    bool IsCleared() const { return isCleared_; }

    Vector3 GetPosition() const;
    float GetRadius() const { return radius_; }

private:
    std::unique_ptr<Object3d> object_;
    std::unique_ptr<RailMover> railMover_;
    
    float radius_ = 1.5f;   // 当たり判定の大きさ
    bool isCleared_ = false; // クリアフラグ
};