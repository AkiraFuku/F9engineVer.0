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

     

    void SetRail(RailPath* rail);
    void Move(float ratio); // 進行させるメソッド

private:
    std::unique_ptr<Object3d> object_;
    std::unique_ptr<RailMover> railMover_; // unique_ptrに変更
    Camera* camera_ = nullptr;

    const float kMoveSpeed_ = 0.1f;
    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };
};