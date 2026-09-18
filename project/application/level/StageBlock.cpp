#include "StageBlock.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include <numbers>

void StageBlock::Initialize(const LevelObjectData& data, Camera* camera)
{
    name_     = data.name;
    type_     = data.type;
    disabled_ = data.disabled;

    // モデルの読み込みと Object3d 初期化
    if (!data.modelName.empty()) {
        if (data.modelName == "box") {
            ModelManager::GetInstance()->CreateBoxModel("box");
        } else {
            ModelManager::GetInstance()->LoadModel(data.modelDir, data.modelName);
        }
    }

    object_ = std::make_unique<Object3d>();
    object_->Initialize();

    if (!data.modelName.empty()) {
        object_->SetModel(data.modelName);
    }

    // テクスチャ設定
    if (!data.texturePath.empty()) {
        TextureManager::GetInstance()->LoadTexture(data.texturePath);
        object_->SetTexture(data.texturePath);
    }

    object_->SetCamera(camera);
    object_->SetTranslate(data.transform.translation);
    object_->SetRotate(data.transform.rotation);
    object_->SetScale(data.transform.scale);
    object_->Update();

    // 初回の三角形構築
    if (!disabled_ && isCollisionEnabled_) {
        triangles_ = object_->GetWorldTriangles();
    }
}

void StageBlock::Update()
{
    if (disabled_) return;

    object_->Update();

    // ワールド三角形を毎フレーム再取得（動的移動に対応）
    if (isCollisionEnabled_) {
        triangles_ = object_->GetWorldTriangles();
    } else {
        triangles_.clear();
    }
}

void StageBlock::Draw()
{
    if (disabled_) return;
    object_->Draw();
}
