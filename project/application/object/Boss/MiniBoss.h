#pragma once
#include "Enemy.h"
#include <vector>
class Cllider;
class BossPart;

class MiniBoss:public GameObject {
public:
    MiniBoss();
    ~MiniBoss();

    void Initialize();
    void Update();
    void Draw();

    void TakeDamage(int damage); // ボス全体のHP減算などのインターフェース
    // シーンから全パーツのコライダーを登録できるように公開
    const std::vector<std::unique_ptr<BossPart>>& GetParts() const {
        return parts_;
    }

    std::vector<Collider*> GetColliders() const;
    //  void OnCollision(GameObject* other) override;
    Vector3 GetPosition() const {
        return position_;
    }
private:
    std::vector<std::unique_ptr<BossPart>> parts_;
    int hp_ = 5;

  //  bool isDamaged_ = false;
    Vector3 position_ = {};

    float deltaTime_ = DXCommon::kDeltaTime; // フレームレートに合わせたデルタタイム

  //  std::unique_ptr<RailMover> railMover_; // unique_ptrに変更

        Camera* camera_ = nullptr;
            const float kMoveSpeed_ = 6.0f;
   // 物理・移動関連の変数（Playerを参考に）
    bool isGrounded_ = true;
    float worldY_ = 0.0f;
    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };
    float gravityScale_ = 1.0f; // 重力のスケール
    const float kGravity = -50.0f;           // 重力加速度（毎フレーム引く値）


    void UpdatePhysics(); // 重力やレール座標の合成処理

      bool isDamaged_ = false;              // クールダウン中かどうかのフラグ
    float hitInvincibilityTimer_ = 0.0f;     // クールダウンタイマー
    const float kHitInvincibilityDuration_ = 1.0f; // クールダウン時間（秒単位にする場合はUpdateの計算に合わせる）
    // パーティクルエミッタの保持
  //  std::unique_ptr<ParticleEmitter> hitParticle_;
      // パーティクルを発生させるヘルパー関数
    void PlayHitEffect();
     Scene* scene_ = nullptr; // Enemyが所属するシーンへのポインタ

    GameObject::GroundRayPalamata rayHitPalamata_;
    const float kHeightOffset = 0.5f; // プレイヤーの高さオフセット（地面からの距離）
};