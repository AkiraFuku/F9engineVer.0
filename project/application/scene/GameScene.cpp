#include "GameScene.h"
#include "ModelManager.h"
#include "Input.h"
#include "imgui.h"
#include "PrimitiveDrawer.h"
#include "SceneManager.h"
#include "ParticleManager.h"//フレームワークに移植
#include "PSOManager.h"
#include "LightManager.h"

#include "RailPath.h"
#include "Object3d.h"

#include "Animation.h"
#include"Audio.h"
#include "TextureManager.h"
#include "RailPath.h"

void GameScene::Initialize() {

    // 1. メインカメラの生成
    auto mainCamera = std::make_unique<Camera>();
    mainCamera->SetTranslate({ 0.0f, 0.0f, -5.0f });

    cameraMap_["Main"] = std::move(mainCamera);

    // 2. デバッグ用カメラの生成
    auto debugCamera = std::make_unique<Camera>();
    debugCamera->SetTranslate({ 0.0f, 10.0f, -20.0f });
    cameraMap_["Debug"] = std::move(debugCamera);

    // 3. 最初はメインカメラをセット
    ChangeActiveCamera(cameraMap_["Main"].get());


    handle_ = Audio::GetInstance()->LoadAudio("resources/fanfare.mp3");

    Audio::GetInstance()->PlayAudio(handle_, true);

    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

    ParticleManager::GetInstance()->CreateParticleGroup("Test", "resources/uvChecker.png");
    LightManager::GetInstance()->AddDirectionalLight({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, 1.0f);
    /*   std::vector<Sprite*> sprites;
       for (uint32_t i = 0; i < 5; i++)
       {*/
    sprite = std::make_unique<Sprite>();
    // sprite->Initialize(spritecommon,"resources/monsterBall.png");
    sprite->Initialize("resources/monsterBall.png");

    sprite->SetPosition(Vector2{ 25.0f + 100.0f,100.0f });
    // sprite->SetSize(Vector2{ 100.0f,100.0f });
    //sprites.push_back(sprite);
   // sprite->SetBlendMode(BlendMode::Add);
    sprite->SetAnchorPoint(Vector2{ 0.5f,0.5f });

    //}



    animation = std::make_unique<Animation>();

    animation->Initialize("resources/AnimatedCube", "AnimatedCube.gltf");
    animation->SetCurrentTime(0.0f);

    skyBox = std::make_unique<SkyBox>();
    skyBox->Initialize();
    skyBox->SetCamera(activeCamera_);
    skyBox->SetTextureByFilePath("resources/output_skybox.dds");

    Object3dCommon::GetInstance()->SetDefaultSkyBox(skyBox.get());


    ModelManager::GetInstance()->LoadModel("resources/AnimatedCube", "AnimatedCube.gltf");
    ModelManager::GetInstance()->CreateSphereModel("sphere");
    object3d = std::make_unique<Object3d>();
    object3d->Initialize();
    object3d->SetModel("sphere");
    object3d->SetCamera(activeCamera_);





    player = std::make_unique<Player>();
    player->Initialize();
    player->SetCamera(activeCamera_);

    player->SetPosition({ 0.0f,0.0f,0.0f });


    cameraController = std::make_unique<CameraController>();
    cameraController->Initialize(cameraMap_["Main"].get());
    cameraController->SetTarget(player.get());
    cameraRail = std::make_unique<RailPath>();

    cameraRail->AddPoint({ 0.0f, 0.0f, -5.0f });
    cameraRail->AddPoint({ 25.0f, 5.0f, -5.0f });
    cameraRail->AddPoint({ 50.0f, 0.0f, -5.0f });

    cameraController->SetRailPath(cameraRail.get());

    debugCameraC = std::make_unique<CameraController>();
    debugCameraC->Initialize(cameraMap_["Debug"].get());
    debugCameraC->SetTarget(player.get());


    stageRail = std::make_unique<RailPath>();
    stageRail->AddPointCR({ 0.0f,0.0f,0.0f });

    stageRail->AddPoint({ 25.0f, 0.0f, 2.5f });
    stageRail->AddPoint({ 50.0f, 0.0f, 0.0f });
    player->SetRail(stageRail.get());






}
void GameScene::Finalize() {

    ParticleManager::GetInstance()->ReleaseParticleGroup("Test");
}
void GameScene::Update() {

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

    if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {



        // Aボタンを押したときの処理

        if (Audio::GetInstance()->IsPlaying(handle_))
        {

            Audio::GetInstance()->StopAudio(handle_);
        }

        //  GetSceneManager()->ChangeScene("GameScene");

    }
    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_DPAD_RIGHT))
    {
        Vector3 camreaTranslate = activeCamera_->GetRotate();
        camreaTranslate = Add(camreaTranslate, Vector3{ 0.0f,1 / 60.0f,0.0f });
        activeCamera_->SetRotate(camreaTranslate);

    }

    //マウスホイールの入力取得

    if (Input::GetInstance()->GetMouseMove().z)
    {
        Vector3 camreaTranslate = cameraMap_["Main"]->GetTranslate();
        camreaTranslate = Add(camreaTranslate, Vector3{ 0.0f,0.0f,static_cast<float>(Input::GetInstance()->GetMouseMove().z) * 0.1f });
        cameraMap_["Main"]->SetTranslate(camreaTranslate);

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

    cameraController->Update();
    debugCameraC->Update();

    activeCamera_->Update();

    stageRail->Update();

    player->Update();


    skyBox->SetTranslate(activeCamera_->GetTranslate());
    skyBox->Update();

    //activeCamera_->UpdateViewProjection();
    object3d->Update();

#ifdef USE_IMGUI
    ImGui::Begin("Debug");

    ImGui::Text("Sprite");
    Vector2 Position =
        sprite->GetPosition();
    ImGui::SliderFloat2("Position", &(Position.x), 0.1f, 1000.0f);
    sprite->SetPosition(Position);


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



    ImGui::End();
#endif // USE_IMGUI

    //sprite->SetRotation(sprite->GetRotation() + 0.1f);
    sprite->Update();
    LightManager::GetInstance()->Update();

}
void GameScene::Draw() {

    skyBox->Draw();
    PrimitiveDrawer::GetInstance()->DrawLine({ 0.0f,0.0f,10.0f }, { 1.5f,1.0f,-10.0f }, { 1.0f,0.0f,0.0f,1.0f });
    PrimitiveDrawer::GetInstance()->DrawLine(position_, { 0.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f });
    PrimitiveDrawer::GetInstance()->DrawTriangle({ 0.0f,0.0f,0.0f }, { 1.0f,0.0f,0.0f }, { 0.0f,1.0f,0.0f }, { 1.0f,1.0f,1.0f,1.0f });
   // PrimitiveDrawer::GetInstance()->Draw();

    Sphere sphere = { 0.0f,0.0f,0.0f,1.0f };
    sphere.rotate = rotation_; // クォータニオンの回転を設定（例: 回転なし）


    PrimitiveDrawer::GetInstance()->DrawSphere(sphere, { 0.0f,1.0f,0.0f,1.0f });
    PrimitiveDrawer::GetInstance()->DrawSphere({ {2.0f,0.0f,0.0f},1.0f ,rotation_}, { 1.0f,0.0f,0.0f,1.0f });
    stageRail->DebugDraw();
    cameraRail->DebugDraw();
    player->Draw();

    ParticleManager::GetInstance()->Draw();
    ///////スプライトの描画
    //sprite->Draw();
   // object3d->Draw();
}
GameScene::GameScene() = default;

GameScene::~GameScene() = default;