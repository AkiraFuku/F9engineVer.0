#include "GameScene.h"
#include "CourseWall.h"
#include "Input.h"

#include "MapChipField.h"
#include "ModelManager.h"
#include "ObstacleFast.h"
#include "ObstacleMax.h"
#include "ObstacleNormal.h"
#include "ObstacleSlow.h"
#include "PSOMnager.h"
#include "Player.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "LightManager.h"
#include <numbers>
#include "imgui.h"
#include "Bitmappedfont.h"
#include "Goal.h"

#include "Sprite.h"

#include "GameContext.h"


// コンストラクタ
GameScene::GameScene() = default;

// デストラクタ
GameScene::~GameScene()
{
    // メモリ解放
    player_.reset();
    playerModel_.reset();
    obstacleFastModel_.clear();
    obstacleFast_.clear();
    obstacleMaxModel_.clear();
    obstacleMax_.clear();
    obstacleNormalModel_.clear();
    obstacleNormal_.clear();
    obstacleSlowModel_.clear();
    obstacleSlow_.clear();
    wallModels_.clear();
    walls_.clear();
}

void GameScene::Initialize() {

    camera = std::make_unique<Camera>();
    camera->SetRotate({ 0.3f, 0.0f, 0.0f });

    camera->SetTranslate({ 0.0f, 0.0f, -5.0f });

    Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());
    ParticleManager::GetInstance()->Setcamera(camera.get());


    handle_ = Audio::GetInstance()->LoadAudio("resources/fanfare.mp3");



    Audio::GetInstance()->PlayAudio(handle_, true);


    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

    LightManager::GetInstance()->AddDirectionalLight({ 1,1,1,1 }, { 0,-1,0 }, 1.0f); // メインライト

    ParticleManager::GetInstance()->CreateParticleGroup(
        "Test", "resources/uvChecker.png");
    /*   std::vector<Sprite*> sprites;
       for (uint32_t i = 0; i < 5; i++)
       {*/
    sprite = std::make_unique<Sprite>();

    sprite->Initialize("resources/uvChecker.png");

    sprite->SetPosition(Vector2{ 25.0f + 100.0f, 100.0f });
    // sprite->SetSize(Vector2{ 100.0f,100.0f });
    // sprites.push_back(sprite);

    sprite->SetBlendMode(BlendMode::Add);
    sprite->SetAnchorPoint(Vector2{ 0.5f, 0.5f });

    //}

    // object3d の初期化
    object3d2 = std::make_unique<Object3d>();
    object3d2->Initialize();

    object3d = std::make_unique<Object3d>();
    object3d->Initialize();

    ModelManager::GetInstance()->LoadModel("plane.obj");
    ModelManager::GetInstance()->LoadModel("axis.obj");
    object3d2->SetTranslate(Vector3{ 0.0f, 10.0f, 0.0f });
    object3d2->SetModel("axis.obj");
    object3d->SetModel("plane.obj");
    ModelManager::GetInstance()->CreateSphereModel("MySphere", 16);
    object3d->SetBlendMode(BlendMode::Add);
    Transform M = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
    emitter = std::make_unique<ParicleEmitter>("Test", M, 10, 5.0f, 0.0f);


    // マップチップフィールドの初期化
    mapChipField_ = std::make_unique<MapChipField>();


    int currentStage = GameContext::GetInstance()->GetStageNum();
    std::string csvPath = "resources/stage1.csv"; // デフォルト

    switch (currentStage) {
    case 1:
      csvPath = "resources/stage1.csv";
      break;
    case 2:
      csvPath = "resources/stage2.csv";
      break;
    case 3:
      csvPath = "resources/stage3.csv";
      break;
    default:
      csvPath = "resources/stage1.csv";
      break;
    }

    mapChipField_->LoadMapChipCsv(csvPath);

    // マップチップの生成
    GenerateFieldObjects();

    // マップチップデータのセット 
    player_->SetMapChipField(mapChipField_.get());

    // ビットマップフォントの生成と初期化
    bitmappedFont_ = std::make_unique<Bitmappedfont>();

   
    // ビットマップフォントのスプライト読み込み (0～9番)
    for (int i = 0; i < 10; ++i) {

        // ファイル名の作成
        std::string filePath = "resources/" + std::to_string(i) + ".png";

    }
// 【修正】ベクターのアドレスを渡す
    bitmappedFont_->Initialize(&bitmappedFontSprites_, camera.get());
     


    // 背景モデルの生成と初期化
    backgroundModel_ = std::make_unique<Object3d>();
    backgroundModel_->SetModel("field.obj");
    backgroundModel_->Initialize();
    backgroundModel_->SetTranslate(Vector3{ 200.0f,0.0f,1400.0f });
    backgroundModel_->SetRadius(2000.0f);

    // テキスト画像の生成と初期化
    clearText_ = std::make_unique<Sprite>();
    clearText_->Initialize("resources/clear.png");
    clearText_->SetPosition(Vector2{ 100.0f, 50.0f });
    gameOverText_ = std::make_unique<Sprite>();
    gameOverText_->Initialize("resources/GameOver.png");
    gameOverText_->SetPosition(Vector2{ 100.0f, 50.0f });
    scoreText_ = std::make_unique<Sprite>();
    scoreText_->Initialize("resources/score.png");
    scoreText_->SetPosition(Vector2{ 50.0f, 300.0f });
    pressSpaceText_ = std::make_unique<Sprite>();
    pressSpaceText_->Initialize("resources/pressSpace.png");
    pressSpaceText_->SetPosition(Vector2{ 100.0f, 3000.0f });


    fade_ = std::make_unique<Fade>();
    fade_->Initialize(camera.get());
    fade_->Start(Fade::Phase::kFadeIn);
}

void GameScene::Finalize() {

    LightManager::GetInstance()->ClearLights();

    ParticleManager::GetInstance()->ReleaseParticleGroup("Test");
}
void GameScene::Update() {

    emitter->Update();

    XINPUT_STATE state;

    // 現在のジョイスティックを取得

    Input::GetInstance()->GetJoyStick(0, state);

    // Aボタンを押していたら

    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {

        // Aボタンを押したときの処理

        GetSceneManager()->ChangeScene("TitleScene");
    }
    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_B)) {
    }


    //// マウスホイールの入力取得

    // if (Input::GetInstance()->GetMouseMove().z) {
    //   Vector3 camreaTranslate = camera->GetTranslate();
    //   camreaTranslate =
    //       Add(camreaTranslate,
    //           Vector3{0.0f, 0.0f,
    //                   static_cast<float>(Input::GetInstance()->GetMouseMove().z)
    //                   *
    //                       0.1f});
    //   camera->SetTranslate(camreaTranslate);
    // }

    if (Input::GetInstance()->GetJoyStick(0, state)) {
        // 左スティックの値を取得
        float x = (float)state.Gamepad.sThumbLX;
        float y = (float)state.Gamepad.sThumbLY;

        // 数値が大きいので正規化（-1.0 ～ 1.0）して使うのが一般的
        float normalizedX = x / 32767.0f;
        float normalizedY = y / 32767.0f;
        Vector3 camreaTranslate = camera->GetTranslate();
        camreaTranslate = Add(camreaTranslate, Vector3{ normalizedX / 60.0f,
                                                       normalizedY / 60.0f, 0.0f });
        camera->SetTranslate(camreaTranslate);
    }

    // クリアしていたら
    if (isCleared_)
    {
        clearText_->Update();
        scoreText_->Update();
        pressSpaceText_->Update();

        scoreBitmappedFonts_.clear();
        SetScore(player_->GetScore(), 10000, 0);
        for (auto& scoreFont : scoreBitmappedFonts_) {
            scoreFont->Update();
        }

        if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE))
        {
            GetSceneManager()->ChangeScene("TitleScene");
        }

    }


    // ゲームオーバーになったら
    if (isGameOver_)
    {
        gameOverText_->Update();
        pressSpaceText_->Update();
        if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE))
        {
            GetSceneManager()->ChangeScene("TitleScene");
        }
    }

    camera->Update();
    object3d->Update();
    object3d2->Update();
    backgroundModel_->Update();

    // プレイヤーの更新処理
    player_->Update(isStarted_);

    // 障害物の更新処理
    for (auto& obstacle : obstacleSlow_) {
        obstacle->Update();
    }

    for (auto& obstacle : obstacleNormal_) {
        obstacle->Update();
    }

    for (auto& obstacle : obstacleFast_) {
        obstacle->Update();
    }

    for (auto& obstacle : obstacleMax_) {
        obstacle->Update();
    }

    for (auto& wall : walls_) {
        wall->Update();
    }

    // ゴールの更新処理
    for (auto& goal : goals_) {
        goal->Update();

    }

    // 当たり判定
    CheckAllCollisions();


    fade_->Update();

#ifdef USE_IMGUI
    ImGui::Begin("Debug");
    ImGui::Text("Sphere");
    Vector3 pos = object3d->GetTranslate();
    Vector3 scale = object3d->GetScale();
    ImGui::SliderFloat3("Pos", &(pos.x), 0.1f, 1000.0f);
    ImGui::DragFloat3("scale", &(scale.x), 0.1f, 1000.0f);
    object3d->SetTranslate(pos);
    object3d->SetScale(scale);
    if (LightManager::GetInstance()->GetPointLightCount() > 0) {
        ImGui::Begin("Light Setting");

        // 0番目のポイントライトのデータを参照で取得
        // "auto&" にすることで、ここで書き換えた内容が直接LightManager内のデータに反映されます
        auto& pointLight2 = LightManager::GetInstance()->GetPointLight(1);

        // 位置の調整
        ImGui::DragFloat3("Point Light2 Pos", &pointLight2.position.x, 0.1f);

        // 色の調整
        ImGui::ColorEdit4("Point Light2 Color", &pointLight2.color.x);

        // 強度の調整
        ImGui::DragFloat("Point Light2 Intensity", &pointLight2.intensity, 0.1f, 0.0f, 100.0f);

        // 減衰率の調整
        ImGui::DragFloat("Point Light2 Decay", &pointLight2.decay, 0.1f, 0.0f, 10.0f);
        auto& pointLight1 = LightManager::GetInstance()->GetPointLight(0);

        // 位置の調整
        ImGui::DragFloat3("Point Light Pos", &pointLight1.position.x, 0.1f);

        // 色の調整
        ImGui::ColorEdit4("Point Light Color", &pointLight1.color.x);

        // 強度の調整
        ImGui::DragFloat("Point Light Intensity", &pointLight1.intensity, 0.1f, 0.0f, 100.0f);

        // 減衰率の調整
        ImGui::DragFloat("Point Light Decay", &pointLight1.decay, 0.1f, 0.0f, 10.0f);
        ImGui::DragFloat("Point Light rad", &pointLight1.radius, 0.1f, 0.0f, 10.0f);

        ImGui::End();
    }

#endif


    // 障害物の削除処理
    obstacleSlow_.erase(std::remove_if(obstacleSlow_.begin(), obstacleSlow_.end(),
        [](const std::unique_ptr<ObstacleSlow>& obstacle) {
            return obstacle->IsScoreNone();
        }),
        obstacleSlow_.end());

    obstacleNormal_.erase(std::remove_if(obstacleNormal_.begin(), obstacleNormal_.end(),
        [](const std::unique_ptr<ObstacleNormal>& obstacle) {
            return obstacle->IsScoreNone();
        }),

        obstacleNormal_.end());

    obstacleFast_.erase(std::remove_if(obstacleFast_.begin(), obstacleFast_.end(),
        [](const std::unique_ptr<ObstacleFast>& obstacle) {
            return obstacle->IsScoreNone();
        }),

        obstacleFast_.end());

    obstacleMax_.erase(std::remove_if(obstacleMax_.begin(), obstacleMax_.end(),
        [](const std::unique_ptr<ObstacleMax>& obstacle) {
            return obstacle->IsScoreNone();

        }),

        obstacleMax_.end());

    if (!isStarted_)
    {
        if (countdownTimer_ <= 0)
        {
            isStarted_ = true;
        }
        else
        {

            countdownTimer_--;
            bitmappedFont_->SetNumber(countdownTimer_ / 60 + 1);
            bitmappedFont_->SetPosition(Vector2{ 500.0f,200.0f });
            bitmappedFont_->Update();

        }
    }

#ifdef USE_IMGUI
    //ImGui::Begin("Debug");
    //ImGui::Text("Sphere");
    //Vector3 pos = object3d->GetTranslate();
    //Vector3 scale = object3d->GetScale();
    //ImGui::SliderFloat3("Pos", &(pos.x), 0.1f, 1000.0f);
    //ImGui::DragFloat3("scale", &(scale.x), 0.1f, 1000.0f);
    //object3d->SetTranslate(pos);
    //object3d->SetScale(scale);
    //if (LightManager::GetInstance()->GetPointLightCount() > 0) {
    //    ImGui::Begin("Light Setting");

    //    // 0番目のポイントライトのデータを参照で取得
    //    // "auto&" にすることで、ここで書き換えた内容が直接LightManager内のデータに反映されます
    //    auto& pointLight2 = LightManager::GetInstance()->GetPointLight(1);

    //    // 位置の調整
    //    ImGui::DragFloat3("Point Light2 Pos", &pointLight2.position.x, 0.1f);

    //    // 色の調整
    //    ImGui::ColorEdit4("Point Light2 Color", &pointLight2.color.x);

    //    // 強度の調整
    //    ImGui::DragFloat("Point Light2 Intensity", &pointLight2.intensity, 0.1f, 0.0f, 100.0f);

    //    // 減衰率の調整
    //    ImGui::DragFloat("Point Light2 Decay", &pointLight2.decay, 0.1f, 0.0f, 10.0f);
    //    auto& pointLight1 = LightManager::GetInstance()->GetPointLight(0);

    //    // 位置の調整
    //    ImGui::DragFloat3("Point Light Pos", &pointLight1.position.x, 0.1f);

    //    // 色の調整
    //    ImGui::ColorEdit4("Point Light Color", &pointLight1.color.x);

    //    // 強度の調整
    //    ImGui::DragFloat("Point Light Intensity", &pointLight1.intensity, 0.1f, 0.0f, 100.0f);

    //    // 減衰率の調整
    //    ImGui::DragFloat("Point Light Decay", &pointLight1.decay, 0.1f, 0.0f, 10.0f);
    //    ImGui::DragFloat("Point Light rad", &pointLight1.radius, 0.1f, 0.0f, 10.0f);

    //    ImGui::End();
    //}

    #endif

    


#ifdef USE_IMGUI
    //ImGui::Begin("Debug");


    ///* Vector3 direction= object3d->GetDirectionalLightDirection();
    // if(ImGui::DragFloat3("Light Direction", &direction.x)){
    // object3d->SetDirectionalLightDirection(direction);
    // }
    // float intensity= object3d->GetDirectionalLightIntensity();
    // if(ImGui::InputFloat("intensity",&intensity)){
    //  object3d->SetDirectionalLightIntensity(intensity);
    // }*/

    //ImGui::Text("Sprite");
    //Vector2 Position =
    //    sprite->GetPosition();
    //ImGui::SliderFloat2("Position", &(Position.x), 0.1f, 1000.0f);
    //sprite->SetPosition(Position);

    //ImGui::Text("PlayerSpeed");
    //float playerSpeedZ = player_->GetSpeedZ();
    //ImGui::SliderFloat("SpeedZ", &playerSpeedZ, 0.0f, 2.0f);

    //ImGui::Text("Speed Stage");
    //SpeedStage speedStage = player_->GetSpeedStage();
    //ImGui::Text("Current Speed Stage: %d", static_cast<int>(speedStage));


    //ImGui::Text("Score");
    //ImGui::Text("Current Score: %d", player_->GetScore());

    //ImGui::Text("Player");
    //Vector3 playerPos = player_->GetWorldPosition();
    //ImGui::SliderFloat3("Player Pos", &(playerPos.x), 0.0f, 0.0f);

    //ImGui::End();
#endif // USE_IMGUI
#ifdef USE_IMGUI
    //sprite->SetRotation(sprite->GetRotation() + 0.1f);
    sprite->Update();



    //ImGui::End();
#endif // USE_IMGUI

    // sprite->SetRotation(sprite->GetRotation() + 0.1f);
    sprite->Update();

}


void GameScene::Draw() {

    // object3d2->Draw();
    // object3d->Draw();
    // ParticleManager::GetInstance()->Draw();
    backgroundModel_->Draw();

  


    if (!isStarted_)
    {
        bitmappedFont_->Draw();
    }

    if (isCleared_)
    {
        clearText_->Draw();
        scoreText_->Draw();
        pressSpaceText_-> Draw();
    }

    for (auto& scoreFont : scoreBitmappedFonts_)
    {
        scoreFont->Draw();
    }


    if (isGameOver_)
    {
        gameOverText_->Draw();
        pressSpaceText_->Draw();
    }


    // プレイヤーの描画処理

    player_->Draw();


    // 障害物の描画処理
    for (auto& obstacle : obstacleSlow_) {
        obstacle->Draw();
    }


    for (auto& obstacle : obstacleNormal_) {
        obstacle->Draw();
    }

    for (auto& obstacle : obstacleFast_) {
        obstacle->Draw();
    }

    for (auto& obstacle : obstacleMax_) {
        obstacle->Draw();
    }

    for (auto& wall : walls_) {
        wall->Draw();
    }

    for (auto& goal : goals_) {

        goal->Draw();

    }

     fade_->Draw();

    ///////スプライトの描画
    // sprite->Draw();
}




void GameScene::CheckAllCollisions() {

    if (!isStarted_)
    {
        return;
    }

#pragma region 自キャラと障害物(遅い)の当たり判定
    // 判定対象1と2の座標
    AABB aabbPlayer, aabbSlow;

    // 自キャラの座標
    aabbPlayer = player_->GetAABB();

    // 障害物(遅い)の座標
    for (auto& obstacle : obstacleSlow_) {
        aabbSlow = obstacle->GetAABB();

        // 当たり判定
        if (isCollision(aabbPlayer, aabbSlow)) {
            // 衝突応答
            player_->OnCollision(obstacle.get());
            obstacle->OnCollision(player_.get());
        }
    }
#pragma endregion

#pragma region 自キャラと障害物(普通)の当たり判定
    // 判定対象2の座標
    AABB aabbNormal;

    // 障害物(普通)の座標
    for (auto& obstacle : obstacleNormal_) {
        aabbNormal = obstacle->GetAABB();

        // 当たり判定
        if (isCollision(aabbPlayer, aabbNormal)) {
            // 衝突応答
            player_->OnCollision(obstacle.get());
            obstacle->OnCollision(player_.get());
        }
    }

#pragma endregion

#pragma region 自キャラと障害物(速い)の当たり判定
    // 判定対象2の座標
    AABB aabbFast;

    // 障害物(普通)の座標
    for (auto& obstacle : obstacleFast_) {
        aabbFast = obstacle->GetAABB();

        // 当たり判定
        if (isCollision(aabbPlayer, aabbFast)) {
            // 衝突応答
            player_->OnCollision(obstacle.get());
            obstacle->OnCollision(player_.get());
        }
    }

#pragma endregion

#pragma region 自キャラと障害物(最大速度)の当たり判定
    // 判定対象2の座標
    AABB aabbMax;

    // 障害物(普通)の座標
    for (auto& obstacle : obstacleMax_) {
        aabbMax = obstacle->GetAABB();

        // 当たり判定
        if (isCollision(aabbPlayer, aabbMax)) {
            // 衝突応答
            player_->OnCollision(obstacle.get());
            obstacle->OnCollision(player_.get());
        }
    }

#pragma endregion


#pragma region 自キャラとコースの壁の当たり判定
    AABB aabbWall;
    for (auto& wall : walls_) {
        aabbWall = wall->GetAABB();
        if (isCollision(aabbPlayer, aabbWall)) {

            player_->OnCollision(wall.get());
        }
    }
#pragma endregion

#pragma region 自キャラとゴールの当たり判定
    AABB aabbGoal;
    for (auto& goal : goals_) {
        aabbGoal = goal->GetAABB();
        if (isCollision(aabbPlayer, aabbGoal)) {
            player_->OnCollision(goal.get());
        }
    }



#pragma endregion

}

bool GameScene::isCollision(const AABB& aabb1, const AABB& aabb2) {
    if (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x &&
        aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y &&
        aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z) {
        return true;
    }

    return false;

}



void GameScene::GenerateFieldObjects() {
    // 要素数
    uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
    uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

    // 要素数を変更する
    // 列数の設定
    worldTransformObjects.resize(numBlockVirtical);

    for (uint32_t i = 0; i < numBlockVirtical; ++i) {
        // 一列の要素数を設定(横方向のブロック数)
        worldTransformObjects[i].resize(numBlockHorizontal);
    }

    // オブジェクトの生成
    for (uint32_t i = 0; i < numBlockVirtical; ++i) {
        for (uint32_t j = 0; j < numBlockHorizontal; j++) {


            // 現在の座標のチップタイプを取得
            MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);
            Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j, i);

            switch (type)
            {
            case MapChipType::kBlank:
                break;

            case MapChipType::kPlayer:
            {
                assert(player_ == nullptr && "自キャラを二重に配置しようとしています");
                // プレイヤーの初期化
                player_ = std::make_unique<Player>();
                playerModel_ = std::make_unique<Object3d>();
                playerModel_->SetTranslate(pos);
                playerModel_->SetModel("maguro.obj");
                playerModel_->Initialize();
                player_->Initialize(playerModel_.get(), camera.get(), pos);
                player_->SetGameScene(this);
            }
            break;
            }
        }
    }



    for (uint32_t i = 0; i < numBlockVirtical; ++i) {
        for (uint32_t j = 0; j < numBlockHorizontal; j++) {

            // 現在の座標のチップタイプを取得
            MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);
            Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j, i);

            switch (type)
            {
            case MapChipType::kBlank:
                break;


            case MapChipType::kObstacle:
            {
                uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
                switch (subID)
                {
                case 0:
                {
                    // モデルを生成してリストに追加
                    auto model = std::make_unique<Object3d>();
                    model->SetModel("ika.obj");
                    model->Initialize();

                    // 本体を生成してリストに追加
                    auto obstacle = std::make_unique<ObstacleSlow>();
                    obstacle->Initialize(model.get(), camera.get(), pos, player_.get());

                    // vectorに保存
                    obstacleSlowModel_.push_back(std::move(model));
                    obstacleSlow_.push_back(std::move(obstacle));
                }
                break;

                case 1:
                {
                    // 障害物(普通)の初期化
                    // モデルを生成してリストに追加
                    auto model = std::make_unique<Object3d>();
                    model->SetModel("cube.obj");
                    model->Initialize();

                    // 本体を生成してリストに追加
                    auto obstacle = std::make_unique<ObstacleNormal>();
                    obstacle->Initialize(model.get(), camera.get(), pos, player_.get());

                    // vectorに保存
                    obstacleNormalModel_.push_back(std::move(model));
                    obstacleNormal_.push_back(std::move(obstacle));
                }

                break;

                case 2:
                {
                    // 障害物(速い)の初期化
                    // モデルを生成してリストに追加
                    auto model = std::make_unique<Object3d>();
                    model->SetModel("taru.obj");
                    model->Initialize();

                    // 本体を生成してリストに追加
                    auto obstacle = std::make_unique<ObstacleFast>();
                    obstacle->Initialize(model.get(), camera.get(), pos, player_.get());

                    // vectorに保存
                    obstacleFastModel_.push_back(std::move(model));
                    obstacleFast_.push_back(std::move(obstacle));
                }

                break;

                case 3:
                {
                    // 障害物(最大速度)の初期化
                    // モデルを生成してリストに追加
                    auto model = std::make_unique<Object3d>();
                    model->SetModel("ship.obj");
                    model->Initialize();

                    // 本体を生成してリストに追加
                    auto obstacle = std::make_unique<ObstacleMax>();
                    obstacle->Initialize(model.get(), camera.get(), pos, player_.get());

                    // vectorに保存
                    obstacleSlowModel_.push_back(std::move(model));
                    obstacleMax_.push_back(std::move(obstacle));
                }

                break;
                }
                break;


            }

            case MapChipType::kGoal:
            {
                auto goalModel = std::make_unique<Object3d>();
                goalModel->SetModel("goal.obj");
                goalModel->Initialize();
                auto goal = std::make_unique<Goal>();
                goal->Initialize(goalModel.get(), camera.get(), pos);
                goalModels_.push_back(std::move(goalModel));
                goals_.push_back(std::move(goal));
            }
            break;

            case MapChipType::kWallStraight: {
                // 1) モデルを生成してリストに追加
                auto model = std::make_unique<Object3d>();
                model->SetModel("wall.obj"); // ここを壁モデルに（例: "wall.obj" とか）
                model->Initialize();

                // 2) 置く位置を少し上げたいなら（壁が地面に埋まるなら）
                Vector3 wallPos = pos;
                // wallPos.y += 1.0f; // 必要なら

                // 3) 本体を生成してリストに追加
                auto wall = std::make_unique<CourseWall>();
                wall->Initialize(model.get(), camera.get(), wallPos);

                // 4) サイズ調整（マスに合わせる）
                wall->SetScale(Vector3{ 2.0f, 4.0f, 2.0f }); // 好きに調整

                // 5) subID があるなら向きに使える（例）
                // uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
                // if (subID == 1) wall->SetYaw(1.570796f); // 90度

                // 6) vectorに保存
                wallModels_.push_back(std::move(model));
                walls_.push_back(std::move(wall));
            } break;
            case MapChipType::kWallTurn: {
                auto model = std::make_unique<Object3d>();
                model->SetModel("wall.obj");
                model->Initialize();

                Vector3 wallPos = pos; // まず2.0グリッド中心

                const float dx =
                    (MapChipField::kBlockWidth - MapChipField::kTurnWidth) *
                    2.0f; // 0.5
                const float dz =
                    (MapChipField::kBlockHeight - MapChipField::kTurnHeight) *
                    2.0f; // 0.5

                uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);

                switch (subID) {
                case 0:
                    wallPos.x += dx;
                    wallPos.z += dz;
                    break; // 右奥
                case 1:
                    wallPos.x -= dx;
                    wallPos.z += dz;
                    break; // 左奥
                case 2:
                    wallPos.x -= dx;
                    wallPos.z -= dz;
                    break; // 左手前
                case 3:
                    wallPos.x += dx;
                    wallPos.z -= dz;
                    break; // 右手前
                default:
                    break;
                }

                auto wall = std::make_unique<CourseWall>();
                wall->Initialize(model.get(), camera.get(), wallPos);

                wall->SetScale(Vector3{ 2.0f, 4.0f, 2.0f }); // 見た目は小さく

                wallModels_.push_back(std::move(model));
                walls_.push_back(std::move(wall));
            } break;

            }
        }


    }



}

void GameScene::SetScore(int score, int num, int count)
{
    // 0除算防止と、全ての桁（1の位まで）出し切ったら終了
    if (num < 1) return;

    // その桁の数字だけをきれいに抜く
    // 例: score=200, num=100 の時 -> (200 / 100) % 10 = 2
    int scoreDigit = (score / num) % 10;

    auto newFont = std::make_unique<Bitmappedfont>();
    newFont->Initialize(&bitmappedFontSprites_, camera.get());
    newFont->SetNumber(scoreDigit);

    float startX = 400.0f;
    float posY = 300.0f;
    float fontWidth = 64.0f;

    Vector2 position = { startX + (count * fontWidth), posY };
    newFont->SetPosition(position);

    scoreBitmappedFonts_.push_back(std::move(newFont));

    // 次の桁（numを10分の1にする）へ進む。
    // score / num の判定はせず、numが0になるまで必ず回す。
    SetScore(score, num / 10, count + 1);
}


