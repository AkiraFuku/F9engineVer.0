#include "GameScene.h"
#include "ModelManager.h"
#include "Input.h"
#include "imgui.h"
#include "PrimitiveDrawer.h"
#include "SceneManager.h"
#include "ParticleManager.h"//フレームワークに移植
#include "PSOManager.h"
#include "LightManager.h"

void GameScene::Initialize() {

    camera = std::make_unique<Camera>();
    camera->SetRotate({ 0.0f,0.0f,0.0f });
    camera->SetTranslate({ 0.0f,0.0f,-5.0f });
    Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());
    ParticleManager::GetInstance()->Setcamera(camera.get());
    PrimitiveDrawer::GetInstance()->SetCamera(camera.get());

    handle_ = Audio::GetInstance()->LoadAudio("resources/fanfare.mp3");

    Audio::GetInstance()->PlayAudio(handle_, true);

    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

    ParticleManager::GetInstance()->CreateParticleGroup("Test", "resources/uvChecker.png");
    LightManager::GetInstance()->AddDirectionalLight({ 0.0f,-1.0f,0.0f }, { 1.0f,1.0f,1.0f }, 1.0f);
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




    ModelManager::GetInstance()->LoadModel("resources/AnimatedCube", "AnimatedCube.gltf");
    object3d = std::make_unique<Object3d>();
    object3d->Initialize();
    object3d->SetModel("AnimatedCube.gltf");
    object3d->SetCamera(camera.get());

    object3d->SetAnimations(animation.get());

    PrimitiveDrawer::GetInstance()->Initialize();


    skyBox = std::make_unique<SkyBox>();
    skyBox->Initialize();
     skyBox->SetCamera(camera.get());
     skyBox->SetTextureByFilePath("resources/output_skybox.dds");



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
         Vector3 camreaTranslate = camera->GetRotate();
        camreaTranslate = Add(camreaTranslate, Vector3{ 0.0f,1 / 60.0f,0.0f });
        camera->SetRotate(camreaTranslate);

    }

    //マウスホイールの入力取得

    if (Input::GetInstance()->GetMouseMove().z)
    {
        Vector3 camreaTranslate = camera->GetTranslate();
        camreaTranslate = Add(camreaTranslate, Vector3{ 0.0f,0.0f,static_cast<float>(Input::GetInstance()->GetMouseMove().z) * 0.1f });
        camera->SetTranslate(camreaTranslate);

    }
    if (Input::GetInstance()->GetJoyStick(0, state))
    {
        // 左スティックの値を取得
        float x = (float)state.Gamepad.sThumbLX;
        float y = (float)state.Gamepad.sThumbLY;

        // 数値が大きいので正規化（-1.0 ～ 1.0）して使うのが一般的
        float normalizedX = x / 32767.0f;
        float normalizedY = y / 32767.0f;
        Vector3 camreaTranslate = camera->GetRotate();
        camreaTranslate = Add(camreaTranslate, Vector3{ normalizedY / 60.0f,normalizedX / 60.0f,0.0f });
        camera->SetRotate(camreaTranslate);
    }

    
    camera->Update();

    camera->UpdateView();
   
    //skyBox_->Update();



    camera->UpdateViewProjection();
    skyBox->SetTranslate(camera->GetTranslate()); 
    skyBox->Update();

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

    ImGui::End();
#endif // USE_IMGUI

    //sprite->SetRotation(sprite->GetRotation() + 0.1f);
    sprite->Update();
}
void GameScene::Draw() {
 skyBox->Draw();
    PrimitiveDrawer::GetInstance()->DrawLine({ 0.0f,0.0f,10.0f }, { 1.5f,1.0f,-10.0f }, { 1.0f,0.0f,0.0f,1.0f });
    PrimitiveDrawer::GetInstance()->DrawLine(position_, { 0.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f });
    PrimitiveDrawer::GetInstance()->DrawTriangle({ 0.0f,0.0f,0.0f }, { 1.0f,0.0f,0.0f }, { 0.0f,1.0f,0.0f }, { 1.0f,1.0f,1.0f,1.0f });
    PrimitiveDrawer::GetInstance()->Draw();

    Sphere sphere = { {0.0f,0.0f,0.0f},1.0f };
    sphere.rotate = rotation_; // クォータニオンの回転を設定（例: 回転なし）

    PrimitiveDrawer::GetInstance()->DrawSphere(sphere, { 0.0f,1.0f,0.0f,1.0f });

   

    ParticleManager::GetInstance()->Draw();
    ///////スプライトの描画
    //sprite->Draw();
   // object3d->Draw();
}