#include "GameScene.h"
#include "ModelManager.h"
#include "Input.h"
#include "imgui.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "PSOMnager.h"
#include "LightManager.h"
#include <numbers>
void GameScene::Initialize() {

    camera = std::make_unique<Camera>();
    camera->SetRotate({ 0.0f,0.0f,0.0f });
    camera->SetTranslate({ 0.0f,0.0f,-5.0f });
    Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());
    ParticleManager::GetInstance()->Setcamera(camera.get());

    handle_ = Audio::GetInstance()->LoadAudio("resources/fanfare.mp3");

    Audio::GetInstance()->PlayAudio(handle_, true);
    //LightManager::GetInstance()->AddDirectionalLight( { 1,1,1,1 }, { 0,-1,0 }, 1.0f); // メインライト
    //LightManager::GetInstance()->AddDirectionalLight( { 1,1,1,1 }, { 0,-1,0 }, 1.0f); // メインライト
    LightManager::GetInstance()->AddSpotLight({ 1.0f, 1.0f, 1.0f, 1.0f }, { 2.0f, 1.25f, 0.0f }, 4.0f, Normalize({ -1.0f,-1.0f,0.0f }), 7.0f, 2.0f, std::cos(std::numbers::pi_v<float> / 3.0f), 1.0f); // メインライト
    LightManager::GetInstance()->AddSpotLight({ 1.0f, 1.0f, 1.0f, 1.0f }, { 2.0f, 1.25f, 0.0f }, 4.0f, Normalize({ -1.0f,-1.0f,0.0f }), 7.0f, 2.0f, std::cos(std::numbers::pi_v<float> / 3.0f), 1.0f); // メインライト

    Vector3 point1 = { 0,0,0 };
    LightManager::GetInstance()->AddPointLight({ 1.0f, 1.0f, 1.0f, 1.0f }, point1, 4.0f, 2.0f, 0.1f);
    LightManager::GetInstance()->AddPointLight({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0,0,0 }, 4.0f, 2.0f, 0.1f);
        TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

    ParticleManager::GetInstance()->CreateParticleGroup("Test", "resources/uvChecker.png");
    /*   std::vector<Sprite*> sprites;
       for (uint32_t i = 0; i < 5; i++)
       {*/
    sprite = std::make_unique<Sprite>();
    // sprite->Initialize("resources/monsterBall.png");
    sprite->Initialize("resources/uvChecker.png");

    sprite->SetPosition(Vector2{ 25.0f + 100.0f,100.0f });
    // sprite->SetSize(Vector2{ 100.0f,100.0f });
    //sprites.push_back(sprite);

    sprite->SetAnchorPoint(Vector2{ 0.5f,0.5f });

    //}




   // object3d の初期化
    object3d2 = std::make_unique<Object3d>();
    object3d2->Initialize();

    object3d = std::make_unique<Object3d>();
    object3d->Initialize();

    ModelManager::GetInstance()->LoadModel("plane.obj");
    ModelManager::GetInstance()->LoadModel("axis.obj");
    ModelManager::GetInstance()->LoadModel("terrain.obj");
    ModelManager::GetInstance()->CreateSphereModel("MySphere", 16);
    // object3d2->SetTranslate(Vector3{ 0.0f,10.0f,0.0f });
    object3d2->SetModel("terrain.obj");
    object3d->SetModel("MySphere");

    camera->SetTranslate({ 0.0f,0.0f,-10.0f });
    Transform M = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    emitter = std::make_unique<ParicleEmitter>("Test", M, 10, 5.0f, 0.0f);
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

        if (Audio::GetInstance()->IsPlaying(handle_))
        {

            Audio::GetInstance()->StopAudio(handle_);
        }



        GetSceneManager()->ChangeScene("TitleScene");


    }
    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_B))
    {

    }

    //マウスホイールの入力取得

    if (Input::GetInstance()->GetMouseMove().z)
    {
        Vector3 cameraTranslate = camera->GetTranslate();
        cameraTranslate = Add(cameraTranslate, Vector3{ 0.0f,0.0f,static_cast<float>(Input::GetInstance()->GetMouseMove().z) * 0.1f });
        camera->SetTranslate(cameraTranslate);

    }
    /*if (Input::GetInstance()->TriggerMouseDown(0))
    {
        if (!Audio::GetInstance()->IsPlaying(handle_))
        {
            Audio::GetInstance()->PlayAudio(handle_);
        }

    }*/
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
    if (Input::GetInstance()->GetJoyStick(0, state))
    {
        {
            // 左スティックの値を取得
            float x = (float)state.Gamepad.sThumbLX;
            float y = (float)state.Gamepad.sThumbLY;

            // 数値が大きいので正規化（-1.0 ～ 1.0）して使うのが一般的
            float normalizedX = x / 32767.0f;
            float normalizedY = y / 32767.0f;
            Vector3 cameraTranslate = camera->GetTranslate();
            cameraTranslate = Add(cameraTranslate, Vector3{ normalizedX / 60.0f,normalizedY / 60.0f,0.0f });
            camera->SetTranslate(cameraTranslate);
        }
        {
            //// 左スティックの値を取得
            float x = (float)state.Gamepad.sThumbRX;
            float y = (float)state.Gamepad.sThumbRY;
            //// 数値が大きいので正規化（-1.0 ～ 1.0）して使うのが一般的
            float normalizedX = x / 32767.0f;
            float normalizedY = y / 32767.0f;

            Vector3 point = LightManager::GetInstance()->GetSpotLight(0).direction;
            point = Add(point, Vector3{ normalizedX / 60.0f,normalizedY / 60.0f,0.0f });
            LightManager::GetInstance()->SetSpotLightDirection(0, point);

        }
    }

    camera->Update();
    object3d->Update();
    object3d2->Update();


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
        auto& spotLight2 = LightManager::GetInstance()->GetSpotLight(1);

        // 位置の調整
        ImGui::DragFloat3("spot Light2 Pos", &spotLight2.position.x, 0.1f);
        ImGui::DragFloat3("spot Light Pos", &spotLight2.direction.x, 0.1f);

        // 色の調整
        ImGui::ColorEdit4("spot Light2 Color", &spotLight2.color.x);

        // 強度の調整
        ImGui::DragFloat("spot Light2 Intensity", &spotLight2.intensity, 0.1f, 0.0f, 100.0f);

        // 減衰率の調整
        ImGui::DragFloat("spot Light2 Decay", &spotLight2.decay, 0.1f, 0.0f, 10.0f);
        auto& spotLight1 = LightManager::GetInstance()->GetSpotLight(0);

        // 位置の調整
        ImGui::DragFloat3("spot Light Pos", &spotLight1.position.x, 0.1f);
        ImGui::DragFloat3("spot Light ", &spotLight1.direction.x, 0.1f);

        // 色の調整
        ImGui::ColorEdit4("spot Light Color", &spotLight1.color.x);

        // 強度の調整
        ImGui::DragFloat("spot Light Intensity", &spotLight1.intensity, 0.1f, 0.0f, 100.0f);

        // 減衰率の調整
        ImGui::DragFloat("spot Light Decay", &spotLight1.decay, 0.1f, 0.0f, 10.0f);
        ImGui::DragFloat("spot Light rad", &spotLight1.distance, 0.1f, 0.0f, 10.0f);

        ImGui::End();
    }



    /*  if (ImGui::ColorEdit4("LightColor", &lightColor.x)) {

          object3d->SetDirectionalLightColor(lightColor);
      }
      Vector3 direction= object3d->GetDirectionalLightDirection();
      if(ImGui::DragFloat3("Light Direction", &direction.x)){
      object3d->SetDirectionalLightDirection(direction);
      }
      float intensity= object3d->GetDirectionalLightIntensity();
      if(ImGui::InputFloat("intensity",&intensity)){
       object3d->SetDirectionalLightIntensity(intensity);
      }*/
    ImGui::Text("Sprite");
    Vector2 Position =
        sprite->GetPosition();
    ImGui::SliderFloat2("Position", &(Position.x), 0.1f, 1000.0f);
    sprite->SetPosition(Position);

    ImGui::End();
#endif // USE_IMGUI

    //sprite->SetRotation(sprite->GetRotation() + 0.1f);
    sprite->Update();
}
void GameScene::Draw() {
    object3d2->Draw();
    object3d->Draw();
    // ParticleManager::GetInstance()->Draw();
     ///////スプライトの描画
    //sprite->Draw();
}