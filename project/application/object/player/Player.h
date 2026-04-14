#pragma once
#include <memory>
#include "Transform.h"
#include "Object3d.h"
class InputHandler;
class Input;
class Camera;
class RailMover;
class RailPath;
class Player
{
public:
    Player();
    ~Player();

    void Initialize();
    void Uppdate();
    void Draw();

    void SetCamera(Camera* camera) {
        camera_ = camera;
        if (object_) {

            object_->SetCamera(camera);
        }
    }
    void SetRailPosition(const Vector2& position) {
        if (railMover_) {
            // レール上の位置を直接設定するための関数
            // 例えば、レールの全長に対して0.0f～1.0fの範囲で位置を指定する場合など
            // ここでは仮にposition.xを進捗として使用する例を示します
            float progress = position.x; // 進捗をx成分から取得（例）
            railMover_->BindProgress(&progress); // 進捗をRailMoverにバインド 
            object_->SetTranslate({ object_->GetTranslate().x, position.y, object_->GetTranslate().z }); // Yは現在のまま、XZはレール上の位置に設定

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


    void Move(float ratio);
    void Jump();
    void Attack();

private:
    std::unique_ptr<InputHandler> inputHandler_; 
    std::unique_ptr<Object3d> object_;
    const float kMoveSpeed_ = 0.2f; // 好みの速度に調整

    void HandleInput();
    Camera* camera_ = nullptr;

    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f }; // 現在の速度
    float worldY_ = 0.0f;
    const float kGravity = -0.015f;           // 重力加速度（毎フレーム引く値）
    const float kJumpAcceleration = 0.3f;     // ジャンプした瞬間の上昇速度
    bool isGrounded_ = true;
    std::unique_ptr<RailMover> railMover_;

};

