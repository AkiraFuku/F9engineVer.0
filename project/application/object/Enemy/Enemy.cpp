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

void Enemy::SetRailPosition(const Vector2& position)
{
    if (railMover_) {
            // レール上の位置を直接設定するための関数
            // 例えば、レールの全長に対して0.0f～1.0fの範囲で位置を指定する場合など
            // ここでは仮にposition.xを進捗として使用する例を示します
            float progress = position.x; // 進捗をx成分から取得（例）
            railMover_->BindProgress(&progress); // 進捗をRailMoverにバインド 
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