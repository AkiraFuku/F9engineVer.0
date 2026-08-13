#include "GoalObject.h"
#include "RailMover.h"
#include "ModelManager.h"
#include "PrimitiveDrawer.h"
#include "Player.h"
#include "Collider.h"
GoalObject::GoalObject() = default;
GoalObject::~GoalObject() = default;

void GoalObject::Initialize() {
    object_ = std::make_unique<Object3d>();
    // モデルは適宜用意したもの、もしくは仮のものをロード
  //  ModelManager::GetInstance()->LoadModel("resources/goal/", "goal.obj");
    object_->Initialize();
    //  object_->SetModel("goal.obj");

    railMover_ = std::make_unique<RailMover>();
    collider_ = std::make_unique<Collider>();
    collider_->initialize(this, radius_);

}

void GoalObject::SetRail(RailPath* rail) {

    if (railMover_) {
          railMover_->SetPath(rail);
     //   object_->SetTranslate(railMover_->GetCurrentPosition());
    }
}

void GoalObject::SetRailPosition(const Vector2& position) {
    if (railMover_) {
        float progress = position.x; // 進捗をx成分から取得（例）
        railMover_->SetProgress(progress); // 進捗をRailMoverにバインド 
        // 初期座標の更新
        Vector3 railPos = railMover_->GetCurrentPosition();
        object_->SetTranslate({ railPos.x, position.y, railPos.z });
        object_->Update();
    }
}

void GoalObject::SetCamera(Camera* camera) {
   /* if (railMover_) {
        object_->SetTranslate(railMover_->GetCurrentPosition());
    }*/
    if (object_) object_->SetCamera(camera);
}

void GoalObject::Update() {
    if (railMover_) {
        Vector3 pos = railMover_->GetCurrentPosition();
        // SetRailPositionで設定した Y座標（高さ）を維持したい場合、
        // メンバ変数に高さを保持しておく必要があります
        object_->SetTranslate(pos); 
    }
    object_->Update();
}

void GoalObject::Draw() {
    // object_->Draw();

    Vector4 color = isCleared_ ? Vector4{ 0.0f, 1.0f, 0.0f, 1.0f } : Vector4{ 1.0f, 1.0f, 0.0f, 1.0f };

    Sphere collisionSphere = { object_->GetTranslate(), radius_, {0,0,0,1} };
    PrimitiveDrawer::GetInstance()->DrawSphere(collisionSphere, color);
}

void GoalObject::OnCollision(GameObject* other) {
   if (other->GetCategory() == CollisionCategory::Player) {
        isCleared_ = true;
    }

}

