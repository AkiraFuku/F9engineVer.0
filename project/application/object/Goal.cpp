#include "Goal.h"
#include "ModelManager.h"
#include "imgui.h"
#include "Egg.h"
#include "SceneManager.h"

void Goal::Initialize()
{
    object_ = std::make_unique<Object3d>();
    object_->Initialize();

    ModelManager::GetInstance()->LoadModel("resources","player/player.obj");
    object_->SetModel("player/player.obj");

    Vector3 translate = object_->GetTranslate();
    translate.y -= 4.0f; // ゴールの位置を少し下げる
    translate.z -= 2.0f;
    object_->SetTranslate(translate);
}

void Goal::Finalize()
{

}

void Goal::Update()
{
    ImGui::Begin("Goal Window");

    Vector3 scale = object_->GetScale();
    if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 10.0f))
    {
        object_->SetScale(scale);
    }

    Vector3 rotate = object_->GetRotate();
    if (ImGui::DragFloat3("Rotate", &rotate.x, 0.1f, -360.0f, 360.0f))
    {
        object_->SetRotate(rotate);
    }

    Vector3 translate = object_->GetTranslate();
    if (ImGui::DragFloat3("Translate", &translate.x, 0.1f, -100.0f, 100.0f))
    {
        object_->SetTranslate(translate);
    }


    ImGui::End();

    object_->Update();
}

void Goal::Draw()
{
    object_->Draw();
}

void Goal::Clear()
{
    // 卵とゴールの当たり判定
    Vector3 goalPos = object_->GetTranslate();
    Vector3 eggPos = egg_->GetWorldPosition();

    // 卵がゴールの真上にあったら
    if (goalPos.x - 1.0f < eggPos.x && eggPos.x < goalPos.x + 1.0f &&
        goalPos.z - 1.0f < eggPos.z && eggPos.z < goalPos.z + 1.0f) {
        // シーン遷移
        SceneManager::GetInstance()->ChangeScene("TitleScene");
    }
}
