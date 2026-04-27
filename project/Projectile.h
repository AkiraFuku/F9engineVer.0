#pragma once
#include "Vector3.h"
#include "Object3d.h"
#include <memory>

class RailMover;
class RailPath;
class Camera;
/// <summary>
/// プレイヤーが射出する弾の基底クラス
/// </summary>
class Projectile {
public:
    Projectile();
    virtual ~Projectile();

    // 初期化: 走行するレール、開始地点(t)、速度(正なら終点方向、負なら始点方向)
    virtual void Initialize(const RailPath* path, float startT, float speed);
    virtual void Update();
    virtual void Draw();

    bool IsDead() const { return isDead_; }
    Vector3 GetPosition() const;
    float GetRadius() const { return radius_; }
    void OnCollision();
    void SetCamera(Camera* camera) {
        camera_ = camera;
        if (object_) {
            object_->SetCamera(camera);
        }
    }

protected:
    std::unique_ptr<RailMover> railMover_;
    std::unique_ptr<Object3d> object_;
    Camera* camera_ = nullptr;

    float speed_ = 0.0f;
    float radius_ = 0.5f;
    bool isDead_ = false;
    int lifeTimer_ = 180; // 3秒程度(60fps)
};