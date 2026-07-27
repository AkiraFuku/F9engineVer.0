#pragma once
#include <memory>
#include "Transform.h"
#include "Object3d.h"
#include "Behavior/PlayerState.h"
#include "ICollider.h"
#include "Animation.h"

class InputHandler;
class Input;
class Camera;
class RailMover;
class RailPath;
class IPlayerBehavior;
class Robot;
class Enemy;
class Scene;

class Player : public ICollider
{
public:
    Player();
    ~Player();

    void OnCollision(ICollider* other) override;

    Vector3 GetWorldPosition() const override {
        return object_->GetTranslate();
    }
    float GetRadius() const override {
        return Radius;
    }
    CollisionCategory GetCategory() const override {
        return CollisionCategory::Player;
    }

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
    void SetVelocity(const Vector3& velocity) {
        velocity_ = velocity;
    }

    IPlayerBehavior* GetBehavior() {
        return baseState_ ? baseState_->GetBehavior() : nullptr;
    }
    IPlayerState* GetState() {
        return baseState_.get();
    }

    void SetAngle(float angle) {
        playerAngle_ = angle;
    }
    void AddVelocity(Vector3 v);

    void SetRail(RailPath* rail);
    void ChangeState(std::unique_ptr<IPlayerState> newState);
    void ChangeBehavior(std::unique_ptr<IPlayerBehavior> newBehavior);

    void Move(float ratio);
    void Jump();
    void Attack();

    float GetRailProgress() const;
    float GetCurrentDistance() const;
    const RailPath* GetRailPath() const;

    const RailMover* GetRailMover() const {
        return railMover_.get();
    }
    bool IsGround() const {
        return isGrounded_;
    }

    Vector3 GetDirection() const;
    int GetMoveDirection() const;
    void UpdateGravity();
    void RayCastUpdate() override;

    bool IsHit() const {
        return isDamaged_;
    }
    float GetHitVisualTimer() const {
        return hitInvincibilityTimer_;
    }

    InputHandler* GetInputHandler() {
        return inputHandler_.get();
    }
    const char* GetStateName() const;
    const char* GetBehaviorName() const;

    void SetScene(Scene* scene);
    Scene* GetScene() {
        return scene_;
    }

    float GetWorldY() const {
        return worldY_;
    }

    bool IsAlive() const {
        return isAlive_;
    }
    bool IsDead() const {
        return !isAlive_;
    }
    void SetAlive(bool alive) {
        isAlive_ = alive;
    }
    void Die() {
        isAlive_ = false;
    }

    bool IsActive() const {
        return isActive_;
    }
    bool SetActive(bool active) {
        isActive_ = active;
        return isActive_;
    }
    bool IsGrounded() const {
        return isGrounded_;
    }
    bool IsRayHit() const {
        return isRayHit_;
    }

    const Triangle& GetRayHitTriangle() const {
        return rayHitTriangle_;
    }
    int GetHitPoints() const {
        return hitPoints_.value;
    }
    int GetMaxHitPoints() const {
        return hitPoints_.max;
    }

    void SetDeltaTime(float deltaTime = DXCommon::kDeltaTime) {
        deltaTime_ = deltaTime;
    }
    float GetDeltaTime() const {
        return deltaTime_;
    }

    void SetGravityScale(float scale) {
        gravityScale_ = scale;
    }

    void TakeDamage();

private:
    float deltaTime_ = DXCommon::kDeltaTime;

    std::unique_ptr<IPlayerState> baseState_;
    Scene* scene_ = nullptr;
    std::unique_ptr<InputHandler> inputHandler_;
    std::unique_ptr<Object3d> object_;
    std::unique_ptr<Animation> animation;

    const float kMoveSpeed_ = 12.0f;

    void UpdateRailPath();
    void HandleInput();
    void HandleDamage();
    void HandleAlive();

    Camera* camera_ = nullptr;
    void ImGuiDrawDebugInfo();

    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };
    float worldY_ = 0.0f;

    float gravityScale_ = 1.0f;
    const float kGravity = -50.0f;
    const float kJumpAcceleration = 24.0f;
    bool isGrounded_ = true;

    ICollider::GroundRayPalamata rayHitPalamata_;
    const float kHeightOffset = 0.5f;

    std::unique_ptr<RailMover> railMover_;
    float playerAngle_ = -10.0f;

    float Radius = 1.0f;
    bool isDamaged_ = false;
    Gauge hitPoints_ = { 3, 3 };
    float hitInvincibilityTimer_ = 0.0f;
    const float kHitInvincibilityDuration_ = 1.5f; // 1.5秒間の無敵時間
    const float kKnockbackForce_ = 0.5f;
    int knockbackDirection_ = 0;
    bool isInvincible_ = false;

    bool isAlive_ = true;
    bool isActive_ = true;
};