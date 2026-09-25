#include "StageManager.h"
#include "Enemy.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "PrimitiveDrawer.h"
#include "PrefabManager.h"
#include "InstancedBlockRenderer.h"
#include <Windows.h>   // OutputDebugStringA

// ─────────────────────────────────────────────────────────────────────
// Load / Hot Reload
// ─────────────────────────────────────────────────────────────────────
void StageManager::Load(const std::string& jsonPath, Camera* camera, SpawnEnemyFunc spawnEnemy)
{
    // プレハブマネージャー & インスタンシングレンダラーの初期化
    PrefabManager::GetInstance()->Initialize();
    InstancedBlockRenderer::GetInstance()->Initialize();

    // 既存リストをクリア（Hot Reload）
    blocks_.clear();
    terrain_.reset();
    props_.clear();
    triggers_.clear();
    ropes_.clear();
    cloths_.clear();
    stageRail_.reset();
    cameraRail_.reset();
    allTriangles_.clear();
    playerSpawnPos_     = { 0.0f, 0.0f, 0.0f };
    playerSpawnRailPos_ = { 0.0f, 0.0f };
    goalPos_            = { 0.0f, 0.0f, 0.0f };
    goalRailPos_        = { 0.0f, 0.0f };

    camera_     = camera;
    loadedPath_ = jsonPath;

    // JSONを読み込む
    LevelData levelData = LevelLoader::Load(jsonPath);

    // 全オブジェクトを再帰的に処理
    for (const auto& obj : levelData.objects) {
        ProcessObject(obj, camera, spawnEnemy);
    }

    // 三角形リストの初期構築
    RebuildTriangles();

    OutputDebugStringA(("[StageManager] Stage loaded: " + jsonPath + "\n").c_str());
}

// ─────────────────────────────────────────────────────────────────────
// 個別オブジェクト処理（再帰）
// ─────────────────────────────────────────────────────────────────────
void StageManager::ProcessObject(const LevelObjectData& data, Camera* camera, SpawnEnemyFunc& spawnEnemy)
{
    // disabled オブジェクトはスキップ
    if (data.disabled) return;

    switch (data.type) {

    // ─── 地形 ────────────────────────────────────────────────────
    case LevelObjectType::kTerrain:
    {
        terrain_ = std::make_unique<Terrain>();
        terrain_->Initialize(data, camera);
        break;
    }

    // ─── ブロック ──────────────────────────────────────────────────
    case LevelObjectType::kBlock:
    {
        auto block = std::make_unique<StageBlock>();
        block->Initialize(data, camera);
        blocks_.push_back(std::move(block));
        break;
    }

    // ─── 背景・装飾（当たり判定なし）──────────────────────────────
    case LevelObjectType::kProp:
    {
        auto prop = std::make_unique<StageBlock>();
        prop->Initialize(data, camera);
        prop->SetCollisionEnabled(false);
        props_.push_back(std::move(prop));
        break;
    }

    // ─── プレイヤー開始位置 ───────────────────────────────────────
    case LevelObjectType::kPlayerSpawn:
        playerSpawnPos_     = data.transform.translation;
        playerSpawnRailPos_ = data.railPos;
        break;

    // ─── ゴール ───────────────────────────────────────────────────
    case LevelObjectType::kGoal:
        goalPos_     = data.transform.translation;
        goalRailPos_ = data.railPos;
        break;

    // ─── 敵 ───────────────────────────────────────────────────────
    case LevelObjectType::kEnemy:
        if (spawnEnemy) {
            spawnEnemy(data);
        }
        break;

    // ─── イベントトリガー ─────────────────────────────────────────
    case LevelObjectType::kTrigger:
    {
        auto trigger = std::make_unique<EventTrigger>();
        trigger->Initialize(data);
        triggers_.push_back(std::move(trigger));
        break;
    }

    // ─── PBD ロープ ───────────────────────────────────────────────
    case LevelObjectType::kPbdRope:
    {
        auto rope = std::make_unique<PbdRope>();
        rope->Initialize(
            data.pbd.startPos,
            data.pbd.endPos,
            data.pbd.numPoints,
            data.pbd.stiffness,
            1.0f / 60.0f,
            data.pbd.damping,
            { 0.0f, data.pbd.gravityY, 0.0f }
        );
        // 固定端の設定
        if (data.pbd.fixStart) rope->SetFixed(0, true);
        if (data.pbd.fixEnd)   rope->SetFixed(data.pbd.numPoints - 1, true);

        ropes_.push_back(std::move(rope));
        break;
    }

    // ─── PBD 布 ───────────────────────────────────────────────────
    case LevelObjectType::kPbdCloth:
    {
        auto cloth = std::make_unique<PbdCloth>();
        cloth->Initialize(
            data.pbd.startPos,
            data.pbd.endPos,
            data.pbd.widthPoints,
            data.pbd.heightPoints,
            data.pbd.stiffness,
            1.0f / 60.0f,
            data.pbd.damping,
            { 0.0f, data.pbd.gravityY, 0.0f }
        );
        cloth->SetCamera(camera);
        if (!data.pbd.texturePath.empty()) {
            TextureManager::GetInstance()->LoadTexture(data.pbd.texturePath);
            cloth->SetTexture(data.pbd.texturePath);
        }
        cloths_.push_back(std::move(cloth));
        break;
    }

    // ─── ステージレール ───────────────────────────────────────────
    case LevelObjectType::kRailStage:
    {
        stageRail_ = std::make_unique<RailPath>();
        stageRail_->SetLoop(data.railLoop);
        for (const auto& rp : data.railPoints) {
            // handleOut/handleIn は "このポイントから出るオフセット"
            Vector3 offsetOut = { rp.handleOut.x - rp.position.x,
                                  rp.handleOut.y - rp.position.y,
                                  rp.handleOut.z - rp.position.z };
            Vector3 offsetIn  = { rp.handleIn.x - rp.position.x,
                                  rp.handleIn.y - rp.position.y,
                                  rp.handleIn.z - rp.position.z };
            stageRail_->AddBezierPoint(rp.position, offsetIn, offsetOut, rp.type);
        }
        stageRail_->Update();
        break;
    }

    // ─── カメラレール ─────────────────────────────────────────────
    case LevelObjectType::kRailCamera:
    {
        cameraRail_ = std::make_unique<RailPath>();
        cameraRail_->SetLoop(data.railLoop);
        for (const auto& rp : data.railPoints) {
            Vector3 offsetOut = { rp.handleOut.x - rp.position.x,
                                  rp.handleOut.y - rp.position.y,
                                  rp.handleOut.z - rp.position.z };
            Vector3 offsetIn  = { rp.handleIn.x - rp.position.x,
                                  rp.handleIn.y - rp.position.y,
                                  rp.handleIn.z - rp.position.z };
            cameraRail_->AddBezierPoint(rp.position, offsetIn, offsetOut, rp.type);
        }
        cameraRail_->Update();
        break;
    }

    default:
        break;
    }

    // 子オブジェクトの再帰処理
    for (const auto& child : data.children) {
        ProcessObject(child, camera, spawnEnemy);
    }
}

// ─────────────────────────────────────────────────────────────────────
// 三角形リスト再構築（全ブロック/地形から集約）
// ─────────────────────────────────────────────────────────────────────
void StageManager::RebuildTriangles()
{
    allTriangles_.clear();
    if (terrain_) {
        const auto& tris = terrain_->GetWorldTriangles();
        allTriangles_.insert(allTriangles_.end(), tris.begin(), tris.end());
    }
    for (const auto& block : blocks_) {
        const auto& tris = block->GetWorldTriangles();
        allTriangles_.insert(allTriangles_.end(), tris.begin(), tris.end());
    }
}

// ─────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────
void StageManager::Update(const Vector3& playerPos)
{
    // 地形更新
    if (terrain_) {
        terrain_->Update();
    }

    // ブロック更新
    for (auto& block : blocks_)   block->Update();
    for (auto& prop  : props_)    prop->Update();

    // 三角形リストを毎フレーム再構築（動的ブロック対応）
    RebuildTriangles();

    // イベントトリガー更新
    for (auto& trigger : triggers_) {
        trigger->Update(playerPos);
    }

    // PBD ロープ更新
    for (auto& rope : ropes_) {
        rope->Update();
    }

    // PBD 布更新
    for (auto& cloth : cloths_) {
        cloth->Update();
    }
}

// ─────────────────────────────────────────────────────────────────────
// Draw
// ─────────────────────────────────────────────────────────────────────
void StageManager::Draw()
{
    // 地形描画（専用PSOによる道/地面テクスチャ割り振り＆穴抜き）
    if (terrain_) {
        terrain_->Draw();
    }

    // ブロック＆装飾オブジェクトの個別描画（各プレハブ固有のテクスチャ・モデルを確実に反映）
    for (auto& block : blocks_)  block->Draw();
    for (auto& prop  : props_)   prop->Draw();

    for (auto& rope  : ropes_)   rope->Draw();
    for (auto& cloth : cloths_)  cloth->Draw();
}

// ─────────────────────────────────────────────────────────────────────
// DebugDraw
// ─────────────────────────────────────────────────────────────────────
void StageManager::DebugDraw()
{
    if (!isDebugDrawEnabled_) return;

    // トリガー領域のワイヤーフレーム
    for (auto& trigger : triggers_) {
        trigger->DebugDraw();
    }

    // レールのデバッグ描画
    if (stageRail_)  stageRail_->DebugDraw();
    if (cameraRail_) cameraRail_->DebugDraw();

    // 接地判定ポリゴンのワイヤーフレーム
    auto* pd = PrimitiveDrawer::GetInstance();
    Vector4 triColor = { 0.0f, 0.8f, 0.0f, 0.6f };
    for (const auto& tri : allTriangles_) {
        pd->DrawLine(tri.vertices[0], tri.vertices[1], triColor);
        pd->DrawLine(tri.vertices[1], tri.vertices[2], triColor);
        pd->DrawLine(tri.vertices[2], tri.vertices[0], triColor);
    }
}
