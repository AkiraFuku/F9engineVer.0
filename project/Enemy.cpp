#include "Enemy.h"
#include "Camera.h"
#include "Object3D.h"
#include "ModelManager.h"
#include "RailMover.h"
#include "RailPath.h"
#include <cmath>

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
}

void Enemy::Update()
{
  /*  if (!railMover_ ) {
        return;
    }

    if (railMover_->GetProgress())
    {

    }*/
    Move(1.0f);

    // 1. レール上の座標を取得
    Vector3 railPos = railMover_->GetCurrentPosition();
    object_->SetTranslate(railPos);

    // 2. 進行方向を向く
    Vector3 dir = railMover_->GetCurrentDirection();
    float angle = std::atan2(dir.x, dir.z);
    object_->SetRotate({ 0.0f, angle, 0.0f });

    object_->Update();
}

void Enemy::Draw()
{
    if (object_) {
        object_->Draw();
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
        railMover_->Advance(ratio * (kMoveSpeed_ / 60.0f));
    }
}