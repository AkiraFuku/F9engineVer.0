#include "Enemy.h"
#include "Camera.h"
#include "Object3D.h"
#include "ModelManager.h"
#include "RailMover.h"
#include "RailPath.h"
#include <cmath>
#include "enemyBehavior.h"
Enemy::Enemy() = default;
Enemy::~Enemy() = default;

void Enemy::Initialize()
{
    object_ = std::make_unique<Object3d>();
    // エネミー用にCubeモデルを作成（既存ならそれを使用）
    ModelManager::GetInstance()->CreateSphereModel("Enemy");
    
    object_->Initialize();
    object_->SetModel("Enemy");
    
    railMover_ = std::make_unique<RailMover>();
    ChangeBehavior(std::make_unique<EnemyBehaviorPatrol>());
}

void Enemy::Update()
{
    // 1. 現在の行動（AI）を更新
    if (behavior_) {
        behavior_->Update(this);
    }

    // 2. 物理計算とレール座標の更新
    UpdatePhysics();

}

void Enemy::Draw()
{
    if (object_) {
        object_->Draw();
    }
}

void Enemy::SetRailPosition(const Vector2& position)
{
    if (railMover_) {
            // レール上の位置を直接設定するための関数
            // 例えば、レールの全長に対して0.0f～1.0fの範囲で位置を指定する場合など
            // ここでは仮にposition.xを進捗として使用する例を示します
            // 進捗をx成分から取得（例）
            railMover_->SetProgress(position.x); // 進捗をRailMoverにバインド 
            object_->SetTranslate({ object_->GetTranslate().x, position.y, object_->GetTranslate().z }); // Yは現在のまま、XZはレール上の位置に設定

        }
}

void Enemy::SetRail(RailPath* rail)
{
    if (rail && railMover_) {
        railMover_->SetPath(rail);
    }
}

void Enemy::Move(float ratio)
{
    if (railMover_) {
        // 毎フレームの移動量を計算して進める
        railMover_->Advance(ratio * (kMoveSpeed_ ));
    }
}
void Enemy::ChangeBehavior(std::unique_ptr<IEnemyBehavior> newBehavior) {
    if (behavior_) behavior_->Finalize(this);
    behavior_ = std::move(newBehavior);
    if (behavior_) behavior_->Initialize(this);
}

void Enemy::UpdatePhysics() {
    // Player::UpdateRailPath() と同様のロジック
    if (!isGrounded_) {
        velocity_.y += kGravity;
    }
    worldY_ += velocity_.y;

    if (worldY_ <= 0.0f) {
        worldY_ = 0.0f;
        velocity_.y = 0.0f;
        isGrounded_ = true;
    }

    Vector3 railPos = railMover_->GetCurrentPosition();
    Vector3 finalPos = { railPos.x, worldY_, railPos.z };
    object_->SetTranslate(finalPos);

    // 回転処理
    Vector3 dir = railMover_->GetCurrentDirection();
    float angle = atan2f(dir.x, dir.z);
    object_->SetRotate({ 0.0f, angle, 0.0f });

    object_->Update();
}
// Enemy.cpp
void Enemy::OnCollision([[maybe_unused]] Player* other) {
    // プレイヤーに当たった時の処理
    // 例：HPを減らす、消滅する、など
}