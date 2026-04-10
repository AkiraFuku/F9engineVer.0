#pragma once
#include <memory>
#include "Transform.h"
#include "Object3d.h"
class Input;
class Camera;
class Player
{
public:
    void Initialize();
    void Uppdate();
    void Draw();

    void SetCamera(Camera* camera) {
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


private:
    Input* input_;

    std::unique_ptr<Object3d> object_;
     const float kMoveSpeed_ = 0.2f; // 好みの速度に調整

    void HandleInput();



};

