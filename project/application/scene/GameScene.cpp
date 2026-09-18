#include "GameScene.h"
#include "application/level/StageManager.h"
#include "ModelManager.h"
#include "Input.h"
#include "imgui.h"
#include "PrimitiveDrawer.h"
#include "SceneManager.h"
#include "ParticleManager.h"//フレームワークに移植
#include "ParticleEmitter.h"
#include "PSOManager.h"
#include "LightManager.h"
#include "RailPath.h"
#include "Object3d.h"
#include "Animation.h"
#include"Audio.h"
#include "TextureManager.h"
#include "RailPath.h"
#include "Enemy.h"
#include "TestEnemy.h"
#include "BoundEnemy.h"
#include "CollisionManager.h"
#include "Projectile.h"
#include "PlayerState.h"
#include <numbers>
#include <Model.h>
#include "GoalObject.h"
#include "Phase.h"
#include "PlayPhase.h"
#include "ClearPhase.h"
#include "defeatPhase.h"
#include "PlayerHPUI.h"
#include "ScoreUI.h"
#include "StartPhase.h"
#include "Fade.h"

void GameScene::Initialize() {

    // 1. メインカメラの生成
    auto mainCamera = std::make_unique<Camera>();
    mainCamera->SetTranslate({ 0.0f, 0.0f, -5.0f });

    cameraMap_["Main"] = std::move(mainCamera);

#ifdef USE_IMGUI
    //// 2. デバッグ・エディタ用カメラの生成
    auto debugCamera = std::make_unique<Camera>();
    debugCamera->SetTranslate({ 0.0f, 10.0f, -20.0f });
    debugCamera->SetEditorMode(true);
    cameraMap_["Debug"] = std::move(debugCamera);
#endif

    // 3. 最初はメインカメラをセット
    ChangeActiveCamera(cameraMap_["Main"].get());


    handle_ = Audio::GetInstance()->LoadAudio("resources/Audio/BGM/bgm_Loop.mp3");

    BGMHandle_=Audio::GetInstance()->PlayAudio(handle_,true,0.25f ,"BGM");
    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("resources/gradationLine.png");
    TextureManager::GetInstance()->LoadTexture("resources/Efect.png");

    ParticleManager::ParticleEmitterFunc initializeFunc = [](const Vector3& emitterPosition, std::mt19937& randomEngine)-> ParticleManager::Particle {

        std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
        std::uniform_real_distribution<float> distTime(1.0f, 10.0f);
        ParticleManager::Particle particle;
        particle.transform.scale = { 1.0f,1.0f,1.0f };
        particle.transform.rotate = { 0.0f,0.0f,0.0f };
        Vector3 randomTranslate = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
        particle.transform.translate = emitterPosition + randomTranslate;
        particle.velocity = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };

        particle.color = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine),1.0f };

        particle.lifeTime = distTime(randomEngine);
        particle.currentTime = 0.0f;
        return particle;
        };
    ParticleManager::ParticleUpdateFunc updateFunc = [](ParticleManager::Particle& particle, float deltaTime) {
        // パーティクルの更新処理
        // 例: 速度に基づいて位置を更新し、寿命を減少させる
        particle.uvTransform.offset.x += deltaTime;
        particle.transform.translate += particle.velocity * deltaTime;
        };
    ParticleManager::ParticleEmitterFunc initialize = [](const Vector3& emitterPosition, std::mt19937& randomEngine)-> ParticleManager::Particle {

        std::uniform_real_distribution<float> rotation(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
        ParticleManager::Particle particle;
        particle.transform.scale = { 0.05f,1.0f,1.0f };
        particle.transform.rotate = { rotation(randomEngine),rotation(randomEngine),rotation(randomEngine) };
        particle.transform.translate = emitterPosition;
        particle.velocity = { 0.0f, 0.0f, 0.0f };

        particle.color = { 1.0f,1.0f,1.0f,1.5f };

        particle.lifeTime = 1.0f;
        particle.currentTime = 0.0f;
        return particle;
        };
    ParticleManager::ParticleUpdateFunc update = [](ParticleManager::Particle& particle, float deltaTime) {

        float progress = particle.currentTime / particle.lifeTime;
        if (progress > 1.0f) progress = 1.0f;

        // 5. イージング（Ease Out）を使って、最初はものすごい勢いで広がり、後半に少し減速させる
        // これにより「ドンッ」と弾けるような勢いが表現できます
        // (1 - (1 - t)^3) は Cubic Ease Out の式です
        float easeOut = 1.0f - std::pow(1.0f - progress, 3.0f);

        // 目標とする最大サイズ
        float maxScale = 2.5f;
        float currentScale = maxScale * easeOut;

        particle.transform.scale = { currentScale, currentScale, currentScale };

        particle.uvTransform.offset.y -= deltaTime * 0.08f; // UVを縦にスクロールさせる
        particle.uvTransform.offset.x -= deltaTime * 0.08f; // UVを縦にスクロールさせる


        // 6. 消え方もイージング（Ease In）をかけるか、後半に一気に消すとキレが出ます
        // 最初はくっきり、後半急激に消えるようにする例（(1 - progress)^2）
        particle.color.w = 1.0f - (progress * progress);


        };

    ParticleManager::GetInstance()->CreateParticleGroup("GameEffects", "Hit", "resources/Efect.png", ParticleManager::EffectType::Ring, initialize, update);
    ParticleManager::GetInstance()->SetCamera(activeCamera_);
    LightManager::GetInstance()->AddDirectionalLight({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, 1.0f);





    animation = std::make_unique<Animation>();

    animation->Initialize("resources/human", "walk.gltf");
    animation->SetCurrentTime(0.0f);

    skyBox = std::make_unique<SkyBox>();
    skyBox->Initialize();
    skyBox->SetCamera(activeCamera_);
    skyBox->SetTextureByFilePath("resources/output_skybox.dds");

    Object3dCommon::GetInstance()->SetDefaultSkyBox(skyBox.get());

    ModelManager::GetInstance()->LoadModel("resources/player/", "playerCursor.obj");

    ModelManager::GetInstance()->LoadModel("resources/AnimatedCube", "AnimatedCube.gltf");
    ModelManager::GetInstance()->LoadModel("resources/simpleSkin", "simpleSkin.gltf");
    ModelManager::GetInstance()->LoadModel("resources/human", "walk.gltf");
    ModelManager::GetInstance()->LoadModel("resources/human", "walk.gltf");
    //  ModelManager::GetInstance()->CreateSphereModel("sphere");
  /*  object3d = std::make_unique<Object3d>();
    object3d->Initialize();
    object3d->SetModel("walk.gltf");

    object3d->SetAnimations(animation.get());
    object3d->SetCamera(activeCamera_);*/

    // --- ステージマネージャーによるJSONステージデータの読み込み ---
    stageManager_ = std::make_unique<StageManager>();
    stageManager_->Load("resources/Stagemap/stage1.json", activeCamera_,
        [this](const LevelObjectData& data) {
            Enemy::EnemyType type = (data.enemyType == "Bound") ? Enemy::EnemyType::Bound : Enemy::EnemyType::Normal;
            AddEnemy(data.railPos, type);
        }
    );

    player = std::make_unique<Player>();
    player->Initialize();
    player->SetCamera(activeCamera_);
    player->SetScene(this);

    player->SetPosition(stageManager_->GetPlayerSpawnPosition());

    playerHPUI_ = std::make_unique<PlayerHPUI>();
    playerHPUI_->Initialize(player.get());

    scoreUI_ = std::make_unique<ScoreUI>();
    scoreUI_->Initialize(player.get());

    cameraController = std::make_unique<CameraController>();
    cameraController->Initialize(cameraMap_["Main"].get());
    cameraController->SetTarget(player.get());

    // カメラレールは StageManager から設定
    if (stageManager_->GetCameraRail()) {
        cameraController->SetRailPath(stageManager_->GetCameraRail());
    }

    debugCameraC = std::make_unique<CameraController>();
    debugCameraC->Initialize(cameraMap_["Debug"].get());
    debugCameraC->SetTarget(player.get());

    RailPath* currentStageRail = GetStageRaill();
    if (currentStageRail) {
        player->SetRail(currentStageRail);
        player->SetRailPosition(stageManager_->GetPlayerSpawnRailPosition());
    }

    // カメラ初期化直後にプレイヤーを追尾・注視するように更新
    cameraController->Update();

    goal_ = std::make_unique<GoalObject>();
    goal_->Initialize();
    goal_->SetCamera(activeCamera_);
    if (currentStageRail) {
        goal_->SetRail(currentStageRail);
        goal_->SetRailPosition(stageManager_->GetGoalRailPosition());
    }


    gaidUI_ = std::make_unique<GaidUI>();
    gaidUI_->Initialize(player.get());

    ChangePhase(std::make_unique<StartPhase>());

}
void GameScene::Finalize() {

    if (Audio::GetInstance()->IsPlaying(BGMHandle_))
    {
        Audio::GetInstance()->StopAudio(BGMHandle_);
    }

}

void GameScene::Update() {
    // CheckClear();

    CheckPhaseTransition();



    debugCameraC->Update();

    activeCamera_->Update();

    UpdateHitStop();
    if (currentPhase_)
    {
        currentPhase_->Update(this);
    }
    // 死んだProjectileを削除
    projectiles_.erase(
        std::remove_if(projectiles_.begin(), projectiles_.end(),
            [](const std::unique_ptr<Projectile>& p) { return p->IsDead(); }),
        projectiles_.end()
    );



    // 死んだ敵を削除
    enemies_.erase(
        std::remove_if(enemies_.begin(), enemies_.end(),
            [](const std::unique_ptr<Enemy>& e) { return e->IsDead(); }),
        enemies_.end()
    );


    skyBox->SetTranslate(activeCamera_->GetTranslate());
    skyBox->Update();

   // object3d->Update();
    if (gaidUI_) {
        gaidUI_->Update();
    }

#ifdef USE_IMGUI
    ImGui::Begin("Debug");

    ImGui::Text("Sprite");



    ImGui::SliderFloat3("Start", &(position_.x), 0.1f, 1000.0f);
    // Vector3 Rotate = camera->GetRotate();
    ImGui::DragFloat4("Rotate", &(rotation_.x));
    // camera->SetRotate(Rotate);


    RailPath* currentStageRail = GetStageRaill();
    if (currentStageRail) {
        Vector3 point1_ = currentStageRail->GetPointPos(1);
        Vector3 point2_ = currentStageRail->GetPointPos(2);

        ImGui::SliderFloat3("Point1", &(point1_.x), -10.0f, 10.0f);
        ImGui::SliderFloat3("Point2", &(point2_.x), -10.0f, 1000.0f);

        currentStageRail->SetPointPos(1, point1_);
        currentStageRail->SetPointPos(2, point2_);
    }

    // ステージリロードボタン（JSON再読み込み）
    if (ImGui::Button("Reload Stage (stage1.json)")) {
        enemies_.clear();
        stageManager_->Load("resources/Stagemap/stage1.json", activeCamera_,
            [this](const LevelObjectData& data) {
                Enemy::EnemyType type = (data.enemyType == "Bound") ? Enemy::EnemyType::Bound : Enemy::EnemyType::Normal;
                AddEnemy(data.railPos, type);
            }
        );
        RailPath* sRail = GetStageRaill();
        if (sRail && player) {
            player->SetRail(sRail);
            player->SetRailPosition(stageManager_->GetPlayerSpawnRailPosition());
        }
        if (sRail && goal_) {
            goal_->SetRail(sRail);
            goal_->SetRailPosition(stageManager_->GetGoalRailPosition());
        }
        if (stageManager_->GetCameraRail() && cameraController) {
            cameraController->SetRailPath(stageManager_->GetCameraRail());
        }
    }

    //カメラを切り替えるボタン
    if (ImGui::Button("Switch Camera")) {
        isDebugCamera_ = !isDebugCamera_;
        if (isDebugCamera_) {
            ChangeActiveCamera(cameraMap_["Debug"].get());
            PrimitiveDrawer::GetInstance()->SetCamera(cameraMap_["Debug"].get());
        } else {
            ChangeActiveCamera(cameraMap_["Main"].get());
        }
    }

    // StateRideOnTestへ切り替えボタン（テスト用）
    if (ImGui::Button("Switch to RideOnTest State")) {
        player->ChangeState(PlayerStateFactory::CreateState(PlayerFormType::Normal));
    }

    // 通常状態へ戻すボタン
    if (ImGui::Button("Switch to Normal State")) {
        player->ChangeState(PlayerStateFactory::CreateState(PlayerFormType::RideOnTest));
    }

    // テスト用：ステートが保持しているアクション情報を表示
    ImGui::Separator();
    ImGui::Text("--- Action Debug ---");

    auto currentState = player->GetState();
    if (currentState) {
        ImGui::Text("State Name: %s", currentState->GetName());

        auto moveAction = currentState->GetMoveAction();
        auto jumpAction = currentState->GetJumpAction();
        auto attackAction = currentState->GetAttackAction_();

        ImGui::Text("MoveAction: %s", moveAction ? "Available" : "Null");
        ImGui::Text("JumpAction: %s", jumpAction ? "Available" : "Null");
        ImGui::Text("AttackAction: %s", attackAction ? "Available" : "Null");

        // テスト用：直接アクション実行ボタン
        if (ImGui::Button("Test: Execute Move Action")) {
            if (moveAction) {
                moveAction->Execute(player.get());
            }
        }

        if (ImGui::Button("Test: Execute Shoot Action")) {
            // RideOnTestの場合のみ射出可能
            IStateRideOn* rideOnState = dynamic_cast<IStateRideOn*>(currentState);
            if (rideOnState) {
                auto shootAction = rideOnState->GetShootAction();
                if (shootAction) {
                    shootAction->Execute(player.get());
                }
            }
        }
    }

    ImGui::End();

    if (currentStageRail) {
        ImGui::Begin("Rail Debug");
        static float r = 20.0f;
        static float hScale = 0.5522f;

        if (ImGui::SliderFloat("Radius", &r, 5.0f, 50.0f) || ImGui::SliderFloat("Handle Scale", &hScale, 0.1f, 1.0f)) {
            // 値が変わったらレールを再構築
            currentStageRail->Initialize(); // points_をクリア
            float h = r * hScale;
            currentStageRail->AddBezierPoint({ 0, 0,  r }, { -h, 0, 0 }, { h, 0, 0 });
            currentStageRail->AddBezierPoint({ r, 0, 0 }, { 0, 0,  h }, { 0, 0, -h });
            currentStageRail->AddBezierPoint({ 0, 0, -r }, { h, 0, 0 }, { -h, 0, 0 });
            currentStageRail->AddBezierPoint({ -r, 0, 0 }, { 0, 0, -h }, { 0, 0,  h });
            currentStageRail->Update();
        }
        ImGui::End();
    }


    if (IsHitStopActive())
    {
        ImGui::Begin("Hit Stop Debug");

        // HitStopの残り時間を表示
        ImGui::Text("Hit Stop Remaining Time: %.2f seconds", stopTimer_);
        // HitStopの残り時間をゲージで表示
        ImGui::ProgressBar(stopTimer_ / 0.1f, ImVec2(0, 0), "Hit Stop Progress");

        ImGui::End();




    }




#endif // USE_IMGUI

    if (stageManager_) {
        stageManager_->Update(player ? player->GetWorldPosition() : Vector3{ 0.0f, 0.0f, 0.0f });
    }

    triangles_.clear();
    if (stageManager_) {
        const auto& stageTris = stageManager_->GetAllTriangles();
        triangles_.insert(triangles_.end(), stageTris.begin(), stageTris.end());
    }

    // 四角いオブジェクトの更新
    if (boxObject_) {
        //プレハブクラスを作るとき更新はここを参考にする
        boxObject_->SetCamera(activeCamera_);
        boxObject_->Update();
        auto boxTris = boxObject_->GetWorldTriangles(); // ワールド変換済み三角形を取得する想定
        triangles_.insert(triangles_.end(), boxTris.begin(), boxTris.end());
    }
    if (TestGround_)
    {
        TestGround_->Update();
        auto testGroundTris = TestGround_->GetWorldTriangles(); // ワールド変換済み三角形を取得する想定
        triangles_.insert(triangles_.end(), testGroundTris.begin(), testGroundTris.end());
    }
    playerHPUI_->Update();
    scoreUI_->Update();

}
void GameScene::Draw() {

    skyBox->Draw();

    if (stageManager_) {
        stageManager_->Draw();
        stageManager_->DebugDraw();
    }
    if (stageRail) stageRail->DebugDraw();
    if (cameraRail) cameraRail->DebugDraw();

    player->Draw();

    if (goal_)
    {
        goal_->Draw();
    }

    // 全てのProjectileを描画
    for (auto& projectile : projectiles_) {
        if (projectile) {
            projectile->Draw();
        }
    }
    // 全ての敵を描画
    for (auto& enemy : enemies_) {
        enemy->Draw();
    }

    ///////スプライトの描画
    //object3d->Draw();

    // 箱オブジェクトの描画
    if (boxObject_) {
        boxObject_->Draw();
    }
    if (TestGround_)
    {
        TestGround_->Draw();
    }

    // --- レイキャストのデバッグ描画 ---
    // 衝突した三角形があれば赤色で強調描画する
    if (player && player->IsRayHit()) {
        hitTriangle_ = player->GetRayHitTriangle();
        PrimitiveDrawer::GetInstance()->DrawTriangle(
            hitTriangle_.vertices[0],
            hitTriangle_.vertices[1],
            hitTriangle_.vertices[2],
            { 1.0f, 0.0f, 0.0f, 0.5f }, // 赤色の半透明
            FillMode::kSolid
        );
        // 輪郭線も描画
        PrimitiveDrawer::GetInstance()->DrawLine(hitTriangle_.vertices[0], hitTriangle_.vertices[1], { 1.0f, 1.0f, 1.0f, 1.0f });
        PrimitiveDrawer::GetInstance()->DrawLine(hitTriangle_.vertices[1], hitTriangle_.vertices[2], { 1.0f, 1.0f, 1.0f, 1.0f });
        PrimitiveDrawer::GetInstance()->DrawLine(hitTriangle_.vertices[2], hitTriangle_.vertices[0], { 1.0f, 1.0f, 1.0f, 1.0f });
    }

    ParticleManager::GetInstance()->Draw();


    playerHPUI_->Draw();
    scoreUI_->Draw();

    if (gaidUI_) {
        gaidUI_->Draw();
    }

    if (currentPhase_)
    {
        currentPhase_->Draw(this);
    }
}
void GameScene::CheckPhaseTransition()
{
    if (goal_)
    {
        if (goal_->IsCleared() && !isCleared_) {
            isCleared_ = true;
            ChangePhase(std::make_unique<ClearPhase>()); // 次のフェーズに遷移
        }
    }
    CheckPlayerFall();
    //プレイヤーが死亡した場合のフェーズ遷移もここでチェックすることができます。
    if (player)
    {
        if (player->IsDead() && !isDefeated_)
        {
            isDefeated_ = true;
            ChangePhase(std::make_unique<defeatPhase>()); // 次のフェーズに遷移
        }
    }
}

void GameScene::CheckPlayerFall()
{

    if (player)
    {
        Vector3 playerPos = player->GetWorldPosition();
        if (playerPos.y < fallLimit_ && !isDefeated_)
        {
            isDefeated_ = true;
            // プレイヤーが落下限界を下回った場合の処理
            player->Die(); // プレイヤーを死亡状態にする
            ChangePhase(std::make_unique<defeatPhase>()); // 次のフェーズに遷移
        }
    }

}
GameScene::GameScene() = default;

GameScene::~GameScene() = default;

void GameScene::AddEnemy(Vector2 pos, Enemy::EnemyType enemyType)
{
    RailPath* rail = GetStageRaill();
    if (rail)
    {// 新しい敵を生成（テスト用敵）
        std::unique_ptr<Enemy> newEnemy = nullptr;

        // 1. タイプに応じて生成する派生クラスを切り替える (ファクトリー処理)
        switch (enemyType)
        {
        case Enemy::EnemyType::Normal:
            newEnemy = std::make_unique<TestEnemy>();
            break;

        case Enemy::EnemyType::Bound:
            newEnemy = std::make_unique<BoundEnemy>();
            break;

        default:
            newEnemy = std::make_unique<TestEnemy>();
            break;
        }

        newEnemy->Initialize();

        // 共通の設定
        newEnemy->SetCamera(cameraMap_["Main"].get());
        newEnemy->SetRail(rail);
        newEnemy->SetScene(this); // SetRailPosition 内で scene_ のレイキャストを使うため先に設定
        newEnemy->SetRailPosition(pos);

        // ベクターに追加
        enemies_.push_back(std::move(newEnemy));
    }

}
// GameScene.cpp

void GameScene::AddProjectile(const Projectile::ProjectileSpawnParam& param, Projectile::ProjectileOwner owner) {
    RailPath* rail = GetStageRaill();
    if (rail) {
        std::unique_ptr<Projectile> newProjectile = std::make_unique<Projectile>();

        // --- 修正箇所 ---
        // 以前はここで direction.x しか見ていませんでしたが、
        // param 自体を Initialize に渡すことで y 方向（高度）の速度も反映させます。

        newProjectile->SetCamera(cameraMap_["Main"].get());

        // Projectile側のInitializeにparamを丸ごと渡す
        newProjectile->Initialize(rail, param, owner);

        projectiles_.push_back(std::move(newProjectile));
    }
}

void GameScene::AddTriangles(std::vector<Triangle> triangles)
{
    triangles_.insert(triangles_.end(), triangles.begin(), triangles.end());
}

void GameScene::CheckClear()
{
    // 2. シーン遷移の実行
    if (isCleared_) {


        // 次のシーン（例: TitleScene や ResultScene）へ遷移（仮）
        GetSceneManager()->ChangeScene("TitleScene");
        return; // 遷移が決まったら以降の更新は不要
    }
}

void GameScene::UpdateHitStop()
{
    if (stopTimer_ > 0) {
        stopTimer_ -= DXCommon::kDeltaTime;
    }
}

RailPath* GameScene::GetStageRaill()
{
    if (stageManager_ && stageManager_->GetStageRail()) {
        return stageManager_->GetStageRail();
    }
    return stageRail.get();
}
