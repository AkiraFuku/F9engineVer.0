#include "StageBlock.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "PrefabManager.h"
#include <numbers>

void StageBlock::Initialize(const LevelObjectData& data, Camera* camera)
{
    name_     = data.name;
    type_     = data.type;
    disabled_ = data.disabled;

    // ─── プレハブ定義の検索とデフォルト解決 ───
    PrefabManager* prefabMgr = PrefabManager::GetInstance();
    const BlockPrefabDefinition* prefabDef = nullptr;

    // 1. properties["prefab"] から検索
    auto propPrefabIt = data.properties.find("prefab");
    if (propPrefabIt != data.properties.end()) {
        prefabDef = prefabMgr->FindPrefab(propPrefabIt->second);
    }
    // 2. modelName から検索
    if (!prefabDef && !data.modelName.empty()) {
        prefabDef = prefabMgr->FindPrefab(data.modelName);
    }
    // 3. name から推測検索（例: "OneWayPlatform" または "Island"）
    if (!prefabDef) {
        if (data.name.find("OneWay") != std::string::npos || data.name.find("oneway") != std::string::npos) {
            prefabDef = prefabMgr->FindPrefab("oneway_platform");
        } else if (data.name.find("Island") != std::string::npos || data.name.find("island") != std::string::npos) {
            prefabDef = prefabMgr->FindPrefab("floating_island");
        }
    }

    // プレハブのデフォルト値をベースにし、data の個別プロパティでオーバーライド
    std::string modelName = data.modelName;
    std::string modelDir = data.modelDir;
    std::string texturePath = data.texturePath;
    Vector3 scale = data.transform.scale;
    bool isOneway = false;

    if (prefabDef) {
        prefabId_ = prefabDef->prefabId;
        if (modelName.empty()) modelName = prefabDef->modelName;
        if (modelDir.empty() || modelDir == "resources") modelDir = prefabDef->modelDir;
        if (texturePath.empty()) texturePath = prefabDef->texturePath;
        if (scale.x == 1.0f && scale.y == 1.0f && scale.z == 1.0f && prefabDef->defaultScale.x != 1.0f) {
            scale = prefabDef->defaultScale;
        }
        isOneway = prefabDef->isOneway;
    } else {
        prefabId_ = modelName.empty() ? "box" : modelName;
    }

    // JSON側の properties["is_oneway"] による明示的オーバーライド
    auto onewayIt = data.properties.find("is_oneway");
    if (onewayIt != data.properties.end()) {
        isOneway = (onewayIt->second == "true" || onewayIt->second == "1" || onewayIt->second == "True");
    }
    isOneway_ = isOneway;

    // モデルの読み込みと Object3d 初期化
    if (!modelName.empty()) {
        if (modelName == "box") {
            ModelManager::GetInstance()->CreateBoxModel("box");
        } else if (modelName == "terrain_grid") {
            // ─── 地形グリッドを頂点データから動的生成 ───────────────────
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
                texturePath,
                uvTile);
        } else {
            ModelManager::GetInstance()->LoadModel(modelDir, modelName);
        }
    }

    object_ = std::make_unique<Object3d>();
    object_->Initialize();

    if (!modelName.empty()) {
        object_->SetModel(modelName);
    }

    // テクスチャ設定
    if (!texturePath.empty()) {
        TextureManager::GetInstance()->LoadTexture(texturePath);
        object_->SetTexture(texturePath);
    }

    object_->SetCamera(camera);
    object_->SetTranslate(data.transform.translation);
    object_->SetRotate(data.transform.rotation);
    object_->SetScale(scale);
    object_->Update();

    // 初回の三角形構築
    if (!disabled_ && isCollisionEnabled_) {
        triangles_ = object_->GetWorldTriangles();
        if (isOneway_) {
            for (auto& tri : triangles_) {
                tri.isOneway = true;
            }
        }
    }
}

void StageBlock::Update()
{
    if (disabled_) return;

    object_->Update();

    // ワールド三角形を毎フレーム再取得（動的移動に対応）
    if (isCollisionEnabled_) {
        triangles_ = object_->GetWorldTriangles();
        if (isOneway_) {
            for (auto& tri : triangles_) {
                tri.isOneway = true;
            }
        }
    } else {
        triangles_.clear();
    }
}

void StageBlock::Draw()
{
    if (disabled_) return;
    object_->Draw();
}
