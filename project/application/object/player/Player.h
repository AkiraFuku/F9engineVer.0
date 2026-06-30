#pragma once
#include <memory>
#include "Transform.h"
#include "Object3d.h"
#include "Behavior/PlayerState.h"
#include "ICollider.h"
class InputHandler;
class Input;
class Camera;
class RailMover;
class RailPath;
class IPlayerBehavior;
class Robot;
class Enemy; // Player.h の場合
class Scene;
class Player : public ICollider
{
public:
    Player();
    ~Player();

    //コリジョン
    void OnCollision(ICollider* other) override; // Player側

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
        return baseState_->GetBehavior();
    };
    IPlayerState* GetState() {
        return baseState_.get();
    }
    void SetAngle(float angle) {
        playerAngle_ = angle;
    }
    void AddVelocity(Vector3 v);

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
    bool IsGround()const {
        return isGrounded_;
    }

    // 現在のレールの進行方向ベクトルを返す
    Vector3 GetDirection() const;
    int GetMoveDirection() const;
    void UpdateGravity();
    void RayCastUpdate();
    float groundY_ = 0.0f;


    //プレイヤーの状態を取得するための関数
    bool IsHit() const {
        return isHit_;
    }
    float GetHitVisualTimer() const {
        return hitVisualTimer_;
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
private:

    // --- 状態管理 ---
    std::unique_ptr<IPlayerState> baseState_; // 現在の状態
    Scene* scene_;   // 「通常・攻撃・ジャンプ」
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
    //レイキャスト当たり判定用の変数
    Ray ray_;
    bool isRayHit_ = false;
    Vector3 rayHitPoint_ = {};
    float rayHitDistance_ = 0.0f;
    Triangle rayHitTriangle_ = {};
    RayTriangleCollisionResult result_ = RayTriangleCollisionResult::NoCollision;
    // --- レール移動管理 ---

    std::unique_ptr<RailMover> railMover_;

    float playerAngle_ = 0.0f;

    // --- 被弾表示用 ---

    float Radius = 1.0f;// 当たり判定の半径
    bool isHit_ = false;       // 今当たっているか
    float hitVisualTimer_ = 0.0f;   // 当たった後の表示持続タイマー
    const float kHitVisualDuration = 10.0f; // 何フレーム表示するか
    // --- その他必要なメンバ変数や関数をここに追加 ---
    //playerの当たり判定用の変数

};

