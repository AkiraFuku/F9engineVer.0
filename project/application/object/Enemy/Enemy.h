#pragma once
#include <memory>
#include "Transform.h"
#include "Object3d.h"

class Camera;
class RailMover;
class RailPath;
class IEnemyBehavior; // 前方宣言
class IEnemyState; // 前方宣言
class Player; // Enemy.h の場合
class Robot; // Enemy.h の場合
class ParticleEmitter; // Enemy.h の場合
class Enemy
{
public:
    Enemy();
    virtual ~Enemy();

    virtual void Initialize();
    virtual void Update();
    virtual void Draw();

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
    void SetRailPosition(const Vector2& position);


    void SetRail(RailPath* rail);
    void Move(float ratio); // 進行させるメソッド

    // 行動（Behavior）を切り替えるメソッド
    void ChangeBehavior(std::unique_ptr<IEnemyBehavior> newBehavior);

    // 状態（State）を切り替えるメソッド
    void ChangeState(std::unique_ptr<IEnemyState> newState);

    // プレイヤーと同様に、IsGround() などの判定があると Behavior 側で便利です
    bool IsGround() const {
        return isGrounded_;
    }

    virtual void OnCollision(Player* other); // Enemy側
     void OnCollision(); // Enemy側
    void UpdateGravity(); // 重力の更新処理
    // enemy状態取得
    IEnemyState* GetState() {
        return state_.get();
    }
    // enemy行動ビヘイビア取得
    IEnemyBehavior* GetBehavior() {
        return behavior_.get();
    };
    //状態名称取得
    const char* GetStateName() const;
    // 行動ビヘイビアの名称取得
    const char* GetBehaviorName() const;
    // 敵が死んでいるかどうかを判定
    bool IsDead() const;

    // ロボットを設定するメソッド（外部または派生クラスのInitializeで呼ぶ）
    void SetRobot(std::unique_ptr<Robot> robot);
    Robot* GetRobot() const {
        return robot_.get();
    }

protected:
    std::unique_ptr<Object3d> object_;
    std::unique_ptr<RailMover> railMover_; // unique_ptrに変更
    Camera* camera_ = nullptr;

    const float kMoveSpeed_ = 0.05f;
    std::unique_ptr<IEnemyBehavior> behavior_; // 現在の行動状態
    std::unique_ptr<IEnemyState> state_; // 現在の状態

    // 物理・移動関連の変数（Playerを参考に）
    bool isGrounded_ = true;
    float worldY_ = 0.0f;
    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };
    const float kGravity = -0.015f;

    void UpdatePhysics(); // 重力やレール座標の合成処理

    bool isHit_ = false;              // クールダウン中かどうかのフラグ
    float hitVisualTimer_ = 0.0f;     // クールダウンタイマー
    const float kHitVisualDuration = 1.0f; // クールダウン時間（秒単位にする場合はUpdateの計算に合わせる）
    // 基底クラスで保持するように変更
    std::unique_ptr<Robot> robot_ = nullptr;

   // パーティクルエミッタの保持
    std::unique_ptr<ParticleEmitter> hitParticle_;

    // パーティクルを発生させるヘルパー関数
    void PlayHitEffect();
};