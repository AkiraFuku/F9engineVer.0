#include "Terrain.h"
#include "ModelManager.h"
#include "imgui.h"

void Terrain::Initialize()
{
    object_ = std::make_unique<Object3d>();
    object_->Initialize();

    ModelManager::GetInstance()->LoadModel("resources","terrain/terrain.obj");
    object_->SetModel("terrain/terrain.obj");
}

void Terrain::Finalize()
{

}

void Terrain::Update()
{
    ImGui::Begin("Terrain Window");

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

void Terrain::Draw()
{
    object_->Draw();
}