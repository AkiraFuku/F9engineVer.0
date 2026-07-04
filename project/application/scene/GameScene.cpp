#include "GameScene.h"
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
#include "CollisionManager.h"
#include "Projectile.h"
#include "PlayerState.h"
#include <numbers>
#include <Model.h>
#include "GoalObject.h"
#include "Phase.h"
#include "PlayPhase.h"
void GameScene::Initialize() {

    // 1. メインカメラの生成
    auto mainCamera = std::make_unique<Camera>();
    mainCamera->SetTranslate({ 0.0f, 0.0f, -5.0f });

    cameraMap_["Main"] = std::move(mainCamera);

    //// 2. デバッグ用カメラの生成
    auto debugCamera = std::make_unique<Camera>();
    debugCamera->SetTranslate({ 0.0f, 10.0f, -20.0f });
    cameraMap_["Debug"] = std::move(debugCamera);

    // 3. 最初はメインカメラをセット
    ChangeActiveCamera(cameraMap_["Main"].get());


    handle_ = Audio::GetInstance()->LoadAudio("resources/fanfare.mp3");

   // Audio::GetInstance()->PlayAudio(handle_, true);

    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("resources/gradationLine.png");
    TextureManager::GetInstance()->LoadTexture("resources/Efect.png");

    //ParticleManager::GetInstance()->CreateParticleGroup("Test", "resources/circle2.png");
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
    // ParticleManager::GetInstance()->CreateParticleGroup("GameEffects", "Test", "resources/gradationLine.png", ParticleManager::EffectType::Plane, initializeFunc, updateFunc);

    ParticleManager::GetInstance()->CreateParticleGroup("GameEffects", "Hit", "resources/Efect.png", ParticleManager::EffectType::Ring, initialize, update);
    /*EulerTransform M = { position_,{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    emitter_ = std::make_unique<ParticleEmitter>("Hit", M, 5, 5.0f, 0.0f);*/
    ParticleManager::GetInstance()->SetCamera(activeCamera_);
    LightManager::GetInstance()->AddDirectionalLight({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, 1.0f);


    //}



    animation = std::make_unique<Animation>();

    // animation->Initialize("resources/AnimatedCube", "AnimatedCube.gltf");
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
    object3d = std::make_unique<Object3d>();
    object3d->Initialize();
    object3d->SetModel("walk.gltf");

    object3d->SetAnimations(animation.get());
    object3d->SetCamera(activeCamera_);





    player = std::make_unique<Player>();
    player->Initialize();
    player->SetCamera(activeCamera_);
    player->SetScene(this);

    player->SetPosition({ 0.0f,0.0f,0.0f });


    cameraController = std::make_unique<CameraController>();
    cameraController->Initialize(cameraMap_["Main"].get());
    cameraController->SetTarget(player.get());

    // --- 円形レールの設定例 ---
    cameraRail = std::make_unique<RailPath>();
    // Initialize内
    float playerRadius = 25.0f;
    float cameraRadius = 40.0f; // プレイヤーより遠くに配置
    float cameraHeight = 8.0f;  // 少し高い位置から見下ろす
    float h_cam = cameraRadius * 0.5522f;

    // カメラレール (cameraRail) の構築
    cameraRail->SetLoop(true);
    cameraRail->AddBezierPoint({ 0, cameraHeight,  cameraRadius }, { h_cam, 0, 0 }, { -h_cam, 0, 0 });
    cameraRail->AddBezierPoint({ -cameraRadius, cameraHeight, 0 }, { 0, 0,  h_cam }, { 0, 0, -h_cam });
    cameraRail->AddBezierPoint({ 0, cameraHeight, -cameraRadius }, { -h_cam, 0, 0 }, { h_cam, 0, 0 });
    cameraRail->AddBezierPoint({ cameraRadius, cameraHeight, 0 }, { 0, 0, -h_cam }, { 0, 0,  h_cam });
    cameraRail->Update();

    cameraController->SetRailPath(cameraRail.get());



    debugCameraC = std::make_unique<CameraController>();
    debugCameraC->Initialize(cameraMap_["Debug"].get());
    debugCameraC->SetTarget(player.get());


    // --- 円形レールの設定例 ---
    stageRail = std::make_unique<RailPath>();
    stageRail->SetLoop(false); // ループを有効化

    float radius = 20.0f;       // 円の半径
    float h = radius * 0.5522f; // ハンドルの長さ

    // 【修正版】反時計回りの順序に変更
   // 点0: 前方 (Z+) -> 次は左(X-)へ向かうので、Outは左(-X)方向
    stageRail->AddBezierPoint({ 0, 0,  radius }, { h, 0, 0 }, { -h, 0, 0 });

    // 点1: 左 (X-) -> 次は後方(Z-)へ向かうので、Outは後方(-Z)方向
    stageRail->AddBezierPoint({ -radius, 0, 0 }, { 0, 0,  h }, { 0, 0, -h });

    // 点2: 後方 (Z-) -> 次は右(X+)へ向かうので、Outは右(+X)方向
    stageRail->AddBezierPoint({ 0, 0, -radius }, { -h, 0, 0 }, { h, 0, 0 });

    // 点3: 右 (X+) -> 次は前方(Z+)へ向かうので、Outは前方(+Z)方向
    stageRail->AddBezierPoint({ radius, 0, 0 }, { 0, 0, -h }, { 0, 0,  h });

    // 最後に必ず更新して距離テーブルを作成
    stageRail->Update();

    player->SetRail(stageRail.get());



    // テスト用に敵を生成する場合
    AddEnemy({ 0.1f, 0.0f });
    AddEnemy({ 0.2f, 0.0f });
    AddEnemy({ 0.3f, 0.0f });
    AddEnemy({ 0.4f, 0.0f });

    goal_ = std::make_unique<GoalObject>();
    goal_->Initialize();
    goal_->SetCamera(activeCamera_);
    goal_->SetRail(stageRail.get());
    goal_->SetRailPosition({ 1.0f, 0.0f }); // レールの終端付近に配置

    goal_->SetRailPosition({ stageRail->GetMaxT(), 0.0f }); // レールの終端付近に配置




    CollisionManager::GetInstance()->SetScene(this);

    currentPhase_ = std::make_unique<PlayPhase>();

    // 立方体モデルの登録と、オブジェクトの生成・配置
    ModelManager::GetInstance()->CreateBoxModel("box");
    boxObject_ = std::make_unique<Object3d>();
    boxObject_->Initialize();
    boxObject_->SetModel("box");
    boxObject_->SetCamera(activeCamera_);

    // レール上の第1ポイントの位置に配置 (Yは少し下げて、スケールを大きめにする)
    Vector3 railPoint1 = stageRail->GetPointPos(2);
    boxObject_->SetTranslate({ railPoint1.x, 0.5f, railPoint1.z });
    boxObject_->SetScale({ 4.0f, 4.0f, 4.0f }); // 大きめの箱にする

    boxObject_->Update();   

    GameScene::AddTriangles(boxObject_->GetWorldTriangles());


    ModelManager::GetInstance()->LoadModel("resources/Stagemap", "TentativeStage.obj");
    TestGround_ =std::make_unique<Object3d>();
    TestGround_->Initialize();
   
    TestGround_->SetModel("TentativeStage.obj");
    TestGround_->SetCamera(activeCamera_);
    TestGround_->SetTranslate({ 0.0f, -0.5f, 0.0f });
    TestGround_->SetScale({ 5.0f, 2.5f, 5.0f });    


    GameScene::AddTriangles(TestGround_->GetWorldTriangles());

}
void GameScene::Finalize() {

    ParticleManager::GetInstance()->ReleaseAllParticleGroupSets();
}
void GameScene::Update() {
    CheckClear();
    XINPUT_STATE state;

    // 現在のジョイスティックを取得
    if (Input::GetInstance()->TriggerMouseDown(0))
    {
        if (Audio::GetInstance()->IsPlaying(handle_))
        {
            Audio::GetInstance()->PauseAudio(handle_);
        } else
        {
            Audio::GetInstance()->ResumeAudio(handle_);

        }
    }


    Input::GetInstance()->GetJoyStick(0, state);

    // Aボタンを押していたら

    if (Input::GetInstance()->TriggerKeyDown(DIK_E)) {

        //   emitter_->Emit();

           // Aボタンを押したときの処理
   /*
           if (Audio::GetInstance()->IsPlaying(handle_))
           {

               Audio::GetInstance()->StopAudio(handle_);
           }*/

           //  GetSceneManager()->ChangeScene("GameScene");

    }
    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_DPAD_RIGHT))
    {
        Vector3 cameraTranslate = activeCamera_->GetRotate();
        cameraTranslate = Add(cameraTranslate, Vector3{ 0.0f,1 / 60.0f,0.0f });
        activeCamera_->SetRotate(cameraTranslate);

    }

    //マウスホイールの入力取得

    if (Input::GetInstance()->GetMouseMove().z)
    {
        Vector3 cameraTranslate = cameraMap_["Main"]->GetTranslate();
        cameraTranslate = Add(cameraTranslate, Vector3{ 0.0f,0.0f,static_cast<float>(Input::GetInstance()->GetMouseMove().z) * 0.1f });
        cameraMap_["Main"]->SetTranslate(cameraTranslate);

    }
    if (Input::GetInstance()->GetJoyStick(0, state))
    {
        // 左スティックの値を取得
        float x = (float)state.Gamepad.sThumbLX;
        float y = (float)state.Gamepad.sThumbLY;

        // 数値が大きいので正規化（-1.0 ～ 1.0）して使うのが一般的
        float normalizedX = x / 32767.0f;
        float normalizedY = y / 32767.0f;
        Vector3 cameraTranslate = activeCamera_->GetRotate();
        cameraTranslate = Add(cameraTranslate, Vector3{ normalizedY / 60.0f,normalizedX / 60.0f,0.0f });
        activeCamera_->SetRotate(cameraTranslate);
    }

    debugCameraC->Update();

    activeCamera_->Update();
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

    object3d->Update();

#ifdef USE_IMGUI
    ImGui::Begin("Debug");

    ImGui::Text("Sprite");



    ImGui::SliderFloat3("Start", &(position_.x), 0.1f, 1000.0f);
    // Vector3 Rotate = camera->GetRotate();
    ImGui::DragFloat4("Rotate", &(rotation_.x));
    // camera->SetRotate(Rotate);


    Vector3 point1_ = stageRail->GetPointPos(1);
    Vector3 point2_ = stageRail->GetPointPos(2);

    ImGui::SliderFloat3("Point1", &(point1_.x), -10.0f, 10.0f);
    ImGui::SliderFloat3("Point2", &(point2_.x), -10.0f, 1000.0f);

    stageRail->SetPointPos(1, point1_);
    stageRail->SetPointPos(2, point2_);

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
        player->ChangeState(std::make_unique<StateRideOnTest>());
    }

    // 通常状態へ戻すボタン
    if (ImGui::Button("Switch to Normal State")) {
        player->ChangeState(std::make_unique<StateNormal>());
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

    ImGui::Begin("Rail Debug");
    static float r = 20.0f;
    static float hScale = 0.5522f;

    if (ImGui::SliderFloat("Radius", &r, 5.0f, 50.0f) || ImGui::SliderFloat("Handle Scale", &hScale, 0.1f, 1.0f)) {
        // 値が変わったらレールを再構築
        stageRail->Initialize(); // points_をクリア
        float h = r * hScale;
        stageRail->AddBezierPoint({ 0, 0,  r }, { -h, 0, 0 }, { h, 0, 0 });
        stageRail->AddBezierPoint({ r, 0, 0 }, { 0, 0,  h }, { 0, 0, -h });
        stageRail->AddBezierPoint({ 0, 0, -r }, { h, 0, 0 }, { -h, 0, 0 });
        stageRail->AddBezierPoint({ -r, 0, 0 }, { 0, 0, -h }, { 0, 0,  h });
        stageRail->Update();
    }
    ImGui::End();
#endif // USE_IMGUI


    // 四角いオブジェクトの更新
    if (boxObject_) {
        boxObject_->SetCamera(activeCamera_);
        boxObject_->Update();
    }
    if (TestGround_)
    {
        TestGround_->Update();
    }
    // Update() の boxObject_->Update() の直後に追加
    triangles_.clear();
    if (boxObject_) {
        auto boxTris = boxObject_->GetWorldTriangles(); // ワールド変換済み三角形を取得する想定
        triangles_.insert(triangles_.end(), boxTris.begin(), boxTris.end());
    }
    if (TestGround_) {
        auto testGroundTris = TestGround_->GetWorldTriangles(); // ワールド変換済み三角形を取得する想定
        triangles_.insert(triangles_.end(), testGroundTris.begin(), testGroundTris.end());
    }

//    // プレイヤーの位置から真下にレイキャスト (箱のメッシュとの判定)
//    if (player && boxObject_) {
//        debugRay_.origin = player->GetWorldPosition();
//        debugRay_.diff = { 0.0f, -10.0f, 0.0f }; // 長さ10の下向きレイ
//
//        isBoxHit_ = false;
//        boxHitDistance_ = FLT_MAX;
//
//        // 箱オブジェクトのワールド空間の三角形リストを取得
//        std::vector<Triangle> triangles = boxObject_->GetWorldTriangles();
//
//        for (const auto& tri : triangles) {
//            float dist = 0.0f;
//            Vector3 hitPt = {};
//            if (CheckRayTriangle(debugRay_, tri, &dist, &hitPt)) {
//                // 最も近い衝突面を選択
//                if (dist < boxHitDistance_) {
//                    boxHitDistance_ = dist;
//                    boxHitPoint_ = hitPt;
//                    hitTriangle_ = tri;
//                    isBoxHit_ = true;
//                }
//            }
//        }
//
#ifdef USE_IMGUI
        ImGui::Begin("Raycast Box Debug");
        //ボックスの位置
       boxPoint_ = boxObject_->GetTranslate();
        ImGui::DragFloat3("Box Position", &(boxPoint_.x));
        boxObject_->SetTranslate(boxPoint_);
//        
//        ImGui::Text("Box Hit: %s", isBoxHit_ ? "True" : "False");
//        if (isBoxHit_) {
//            ImGui::Text("Hit Distance: %.3f", boxHitDistance_);
//            ImGui::Text("Hit Point: (%.3f, %.3f, %.3f)", boxHitPoint_.x, boxHitPoint_.y, boxHitPoint_.z);
//        }
        ImGui::End();
#endif
//    }

    //  LightManager::GetInstance()->Update();

}
void GameScene::Draw() {

    skyBox->Draw();

    stageRail->DebugDraw();
    cameraRail->DebugDraw();
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

    ParticleManager::GetInstance()->Draw();
    ///////スプライトの描画
    object3d->Draw();

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
    if (isBoxHit_) {
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

    //// プレイヤーから射出されるレイを描画 (当たっているなら赤、外れているなら緑)
    //Vector4 rayColor = isBoxHit_ ? Vector4{ 1.0f, 0.0f, 0.0f, 1.0f } : Vector4{ 0.0f, 1.0f, 0.0f, 1.0f };
    //Vector3 rayEnd = Add(debugRay_.origin, debugRay_.diff);
    //PrimitiveDrawer::GetInstance()->DrawLine(debugRay_.origin, rayEnd, rayColor);

    //// 衝突点がある場合は、交点に球体を描画
    //if (isBoxHit_) {
    //    Sphere hitSphere = { boxHitPoint_, 0.2f, { 0.0f, 0.0f, 0.0f, 1.0f } };
    //    PrimitiveDrawer::GetInstance()->DrawSphere(hitSphere, { 0.0f, 0.0f, 1.0f, 1.0f }); // 青い球
    //}
}
GameScene::GameScene() = default;

GameScene::~GameScene() = default;

void GameScene::AddEnemy(Vector2 pos)
{
    if (stageRail)
    {// 新しい敵を生成（テスト用敵）
        std::unique_ptr<TestEnemy> newEnemy = std::make_unique<TestEnemy>();
        newEnemy->Initialize();

        // 共通の設定
        newEnemy->SetCamera(cameraMap_["Main"].get());
        newEnemy->SetRail(stageRail.get());
        newEnemy->SetRailPosition(pos);

        // ベクターに追加
        enemies_.push_back(std::move(newEnemy));
    }

}
// GameScene.cpp

void GameScene::AddProjectile(const Projectile::ProjectileSpawnParam& param, Projectile::ProjectileOwner owner) {
    if (stageRail) {
        std::unique_ptr<Projectile> newProjectile = std::make_unique<Projectile>();

        // --- 修正箇所 ---
        // 以前はここで direction.x しか見ていませんでしたが、
        // param 自体を Initialize に渡すことで y 方向（高度）の速度も反映させます。

        newProjectile->SetCamera(cameraMap_["Main"].get());

        // Projectile側のInitializeにparamを丸ごと渡す
        newProjectile->Initialize(stageRail.get(), param, owner);

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
        // 必要に応じてフェードアウト演出やSE再生をここで行う
        if (Audio::GetInstance()->IsPlaying(handle_)) {
            Audio::GetInstance()->StopAudio(handle_);
        }

        // 次のシーン（例: TitleScene や ResultScene）へ遷移（仮）
        GetSceneManager()->ChangeScene("TitleScene");
        return; // 遷移が決まったら以降の更新は不要
    }
}
