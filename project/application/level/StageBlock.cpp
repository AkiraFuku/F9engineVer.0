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
        } else if (data.modelName == "terrain_grid") {
            // ─── 地形グリッドを頂点データから動的生成 ───────────────────
            // propertiesから寸法・分割数を取得（なければデフォルト値を使用）
            auto getFloat = [&](const std::string& key, float def) -> float {
                auto it = data.properties.find(key);
                if (it != data.properties.end()) {
                    try { return std::stof(it->second); }
                    catch (...) {}
                }
                return def;
            };
            auto getInt = [&](const std::string& key, int def) -> int {
                auto it = data.properties.find(key);
                if (it != data.properties.end()) {
                    try { return std::stoi(it->second); }
                    catch (...) {}
                }
                return def;
            };

            float sizeX    = getFloat("size_x",      150.0f);
            float sizeY    = getFloat("size_y",      150.0f);
            int   divX     = getInt  ("divisions_x",   20);
            int   divY     = getInt  ("divisions_y",   20);
            float uvTile   = getFloat("uv_tile",       1.0f);

            ModelManager::GetInstance()->CreateTerrainModel(
                "terrain_grid",
                sizeX, sizeY, divX, divY,
                data.texturePath,
                uvTile);
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
