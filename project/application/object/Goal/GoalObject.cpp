#include "GoalObject.h"
#include "RailMover.h"
#include "ModelManager.h"
#include "PrimitiveDrawer.h"
#include "Player.h"
GoalObject::GoalObject() = default;
GoalObject::~GoalObject() = default;

void GoalObject::Initialize() {
    object_ = std::make_unique<Object3d>();
    // モデルは適宜用意したもの、もしくは仮のものをロード
  //  ModelManager::GetInstance()->LoadModel("resources/goal/", "goal.obj");
    object_->Initialize();
    //  object_->SetModel("goal.obj");

    railMover_ = std::make_unique<RailMover>();
}

void GoalObject::SetRail(RailPath* rail) {
    if (railMover_) railMover_->SetPath(rail);
}

void GoalObject::SetRailPosition(const Vector2& position) {
    if (railMover_) {
        float progress = position.x; // 進捗をx成分から取得（例）
        railMover_->BindProgress(&progress); // 進捗をRailMoverにバインド 
        // 初期座標の更新
        Vector3 railPos = railMover_->GetCurrentPosition();
        object_->SetTranslate({ railPos.x, position.y, railPos.z });
    }
}

void GoalObject::SetCamera(Camera* camera) {
    if (object_) object_->SetCamera(camera);
}

void GoalObject::Update() {
    if (railMover_) {
        object_->SetTranslate(railMover_->GetCurrentPosition());
    }
    object_->Update();
}

void GoalObject::Draw() {
    // object_->Draw();

    Vector4 color = isCleared_ ? Vector4{ 0.0f, 1.0f, 0.0f, 1.0f } : Vector4{ 1.0f, 1.0f, 0.0f, 1.0f };

    Sphere collisionSphere = { object_->GetTranslate(), radius_, {0,0,0,1} };
    PrimitiveDrawer::GetInstance()->DrawSphere(collisionSphere, color);
}

void GoalObject::OnCollision(Player* player) {
    Vector3 pPos = player->GetTransform().translate;
    Vector3 gPos = object_->GetTranslate();
    float dist = Length(pPos - gPos);
    if (dist < (player->GetRadius() + radius_)) {
        isCleared_ = true;
    }

}

Vector3 GoalObject::GetPosition() const {
    return object_->GetTranslate();
}