#pragma once
#include <memory>
#include "Transform.h"
#include "Object3d.h"
class InputHandler;
class Input;
class Camera;
class RailMover;
class RailPath;
class IPlayerState;
class IPlayerBehavior;
class Robot;
class Enemy; // Player.h の場合

class Player
{
public:
    Player();
    ~Player();

    void Initialize();
    void Update();
    void Draw();

    void SetCamera(Camera* camera) {
        camera_ = camera;
        if (object_) {

            object_->SetCamera(camera);
        }
    }
    void SetRailPosition(const Vector2& position);

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
    IPlayerBehavior* GetBehavior() {
        return behavior_.get();
    };

    void SetRail(RailPath* rail);
    // 状態を切り替えるメソッド
    void ChangeState(std::unique_ptr<IPlayerState> newState);
    void ChangeBehavior(std::unique_ptr<IPlayerBehavior> newBehavior);

    void Move(float ratio);
    void Jump();
    void Attack();

    float GetRailProgress() const;
    float GetCurrentDistance() const;
    const RailPath* GetRailPath()const; 
    bool IsGround()const{ return isGrounded_;}

    void OnCollision(Enemy* other); // Player側

private:

    std::unique_ptr<IPlayerState> baseState_; // 現在の状態
    std::unique_ptr<IPlayerBehavior> behavior_;   // 「通常・攻撃・ジャンプ」
    std::unique_ptr<InputHandler> inputHandler_;
    std::unique_ptr<Object3d> object_;
    const float kMoveSpeed_ = 0.2f; // 好みの速度に調整
    void UpdateRailPath();
    void HandleInput();
    Camera* camera_ = nullptr;

    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f }; // 現在の速度
    float worldY_ = 0.0f;
    const float kGravity = -0.015f;           // 重力加速度（毎フレーム引く値）
    const float kJumpAcceleration = 0.4f;     // ジャンプした瞬間の上昇速度
    bool isGrounded_ = true;
    std::unique_ptr<RailMover> railMover_;

};

