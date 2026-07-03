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
        if (!baseState_->GetBehavior())
        {
            return nullptr;
        }
        return baseState_->GetBehavior();
    };
    IPlayerState* GetState() {
        if (!baseState_)
        {
            return nullptr;
        }
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

    // ワールド方向ベクトルを返す
    Vector3 GetDirection() const;
    // レール上の進行方向を返す
    int GetMoveDirection() const;
    //重力の更新処理
    void UpdateGravity();
    //レイキャスト判定処理
    void RayCastUpdate();


    //プレイヤーの状態を取得するための関数
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

private:

    // --- 状態管理 ---
    std::unique_ptr<IPlayerState> baseState_; // 現在のプレイヤーの状態管理のためのポインタ
    Scene* scene_;
    std::unique_ptr<InputHandler> inputHandler_;
    std::unique_ptr<Object3d> object_;
    const float kMoveSpeed_ = 0.2f; // 好みの速度に調整
    void UpdateRailPath();
    void HandleInput();
    //無敵・被弾処理
    void HandleDamage();
    //生存管理
    void HandleAlive();
    //--- カメラ ---
    Camera* camera_ = nullptr;

    void ImGuiDrawDebugInfo();

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
    float groundY_ = 0.0f;

    const float kRayOffset = 2.0f; // レイの始点を上に持ち上げるオフセット
    const float kHeightOffset = 0.5f; // プレイヤーの高さオフセット（地面からの距離）

    // --- レール移動管理 ---

    std::unique_ptr<RailMover> railMover_;

    float playerAngle_ = 0.0f;

    // --- 被弾処理用 ---

    float Radius = 1.0f;// 当たり判定の半径
    bool isDamaged_ = false;      //被弾フラグ
    int32_t hitPoints_ = 3; // プレイヤーの体力
    float hitInvincibilityTimer_ = 0.0f;   // 無敵時間のタイマー
    const float kHitInvincibilityDuration_ = 10.0f; // 無敵時間
    const float kKnockbackForce_ = 0.5f; // ノックバックの強さ
    int knockbackDirection_ = 0; // ノックバックの方向（1:前方、-1:後方）



    //無敵フラグ
    bool isInvincible_ = false;
    // --- その他必要なメンバ変数や関数をここに追加 ---
    //プレイヤーの生存フラグ
    bool isAlive_ = true;
    //プレイヤーの有効フラグ
    bool isActive_ = true;



};

