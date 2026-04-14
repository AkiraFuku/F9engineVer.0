#pragma once
#include <memory>
#include "Transform.h"
#include "Object3d.h"
class Camera;
class RailMover;
class RailPath;
class Enemy
{
    public:
    Enemy();
    ~Enemy();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera) {
        camera_ = camera;
        if (object_) {
            object_->SetCamera(camera);
        }
    }
    void SetPosition(const Vector3& position) {
        if (object_) {
            object_->SetTranslate(position);
        }
    }
    void SetTransform(const EulerTransform& transform) {
        if (object_) {
            object_->SetScale(transform.scale);
            object_->SetRotate(transform.rotate);
            object_->SetTranslate(transform.translate);
        }
    }
    EulerTransform GetTransform() const {
        if (object_) {
            return { object_->GetScale(), object_->GetRotate(), object_->GetTranslate() };
        }
        return {};
    }
    Vector3 GetVelocity() const {
        return velocity_;
    }
    void SetRail(RailPath* rail);

private:
    std::unique_ptr<Object3d> object_;
    const float kMoveSpeed_ = 0.1f; // 好みの速度に調整
    Vector3 velocity_;
    RailMover* railMover_;
    Camera* camera_;
};

