#include "TitleScene.h"
#include "GameScene.h"
#include "Input.h"
#include "ModelManager.h"
#include "PSOMnager.h"
#include "ParticleManager.h" //フレームワークに移植
#include "SceneManager.h"
#include "imgui.h"

void TitleScene::Initialize() {

    camera = std::make_unique<Camera>();
    camera->SetRotate({ 0.0f, 0.0f, 0.0f });
    camera->SetTranslate({ 0.0f, 0.0f, -5.0f });
    Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());
    ParticleManager::GetInstance()->Setcamera(camera.get());

    bgmHandle_ = Audio::GetInstance()->LoadAudio("resources/sounds/title.wav");
    enterHandle_ = Audio::GetInstance()->LoadAudio("resources/sounds/enter.wav");

    Audio::GetInstance()->PlayAudio(bgmHandle_, true);

    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");

    ParticleManager::GetInstance()->CreateParticleGroup(
        "Test", "resources/uvChecker.png");
    /*   std::vector<Sprite*> sprites;
       for (uint32_t i = 0; i < 5; i++)
       {*/
       //sprite = std::make_unique<Sprite>();
       //// sprite->Initialize(spritecommon,"resources/monsterBall.png");
       //sprite->Initialize("resources/monsterBall.png");

       //sprite->SetPosition(Vector2{25.0f + 100.0f, 100.0f});
       //// sprite->SetSize(Vector2{ 100.0f,100.0f });
       //// sprites.push_back(sprite);
       //// sprite->SetBlendMode(BlendMode::Add);
       //sprite->SetAnchorPoint(Vector2{0.5f, 0.5f});

       //}

    ModelManager::GetInstance()->LoadModel("plane.obj");

    // タイトルロゴの初期化
    sprite = std::make_unique<Sprite>();
    sprite->Initialize("resources/backGround.png");
    sprite->SetPosition(Vector2{ 0.0f, 0.0f });
    sprite->SetAnchorPoint(Vector2{ 0.0f, 0.0f });

    titleLogoSprite = std::make_unique<Sprite>();
    titleLogoSprite->Initialize("resources/Title.png");
    titleLogoSprite->SetPosition(Vector2{ 350.0f, 100.0f });
    titleLogoSprite->SetAnchorPoint(Vector2{ 0.0f, 0.0f });

    pressStartSprite = std::make_unique<Sprite>();
    pressStartSprite->Initialize("resources/pressSpaceTitle.png");
    pressStartSprite->SetPosition(Vector2{ 200.0f, 300.0f });
    pressStartSprite->SetAnchorPoint(Vector2{ 0.0f, 0.0f });

    fade_ = std::make_unique<Fade>();
    fade_->Initialize(camera.get());
    fade_->Start(Fade::Phase::kFadeIn);
}
void TitleScene::Finalize() {

    ParticleManager::GetInstance()->ReleaseParticleGroup("Test");
}
void TitleScene::Update() {

    // SPACE 押したら FadeOut 開始
    if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {
        if (!requestSceneChange_) {
            fade_->Start(Fade::Phase::kFadeOut); // FadeOut
            requestSceneChange_ = true;
            Audio::GetInstance()->PlayAudio(enterHandle_, false);
        }
    }

    // タイトルロゴの更新

    // ロゴを上下に揺らす
    theta_ += static_cast<float>(M_PI) / 60.0f;
    titleLogoSprite->SetPosition(Vector2{ 350.0f,sinf(theta_) * amplitude_ });
	

    sprite->Update();
    titleLogoSprite->Update();
    pressStartSprite->Update();


    // フェード更新
    fade_->Update();

    // 画面を覆い切った瞬間にシーン切替
    if (requestSceneChange_ && fade_->IsCovering()) {
        GetSceneManager()->ChangeScene("SelectScene");
        requestSceneChange_ = false;
        Audio::GetInstance()->StopAudio(bgmHandle_);
        return;
    }

    XINPUT_STATE state;

    //// 現在のジョイスティックを取得
    //if (Input::GetInstance()->TriggerMouseDown(0)) {
    //    if (Audio::GetInstance()->IsPlaying(handle_)) {
    //        Audio::GetInstance()->PauseAudio(handle_);
    //    }
    //    else {
    //        Audio::GetInstance()->ResumeAudio(handle_);
    //    }
    //}

    Input::GetInstance()->GetJoyStick(0, state);

    // Aボタンを押していたら

    //if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {

    //  // Aボタンを押したときの処理

    //  if (Audio::GetInstance()->IsPlaying(handle_)) {

    //    Audio::GetInstance()->StopAudio(handle_);
    //  }

    //  GetSceneManager()->ChangeScene("GameScene");
    //}
    //if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_B)) {
    //}

    //// マウスホイールの入力取得

    //if (Input::GetInstance()->GetMouseMove().z) {
    //  Vector3 camreaTranslate = camera->GetTranslate();
    //  camreaTranslate =
    //      Add(camreaTranslate,
    //          Vector3{0.0f, 0.0f,
    //                  static_cast<float>(Input::GetInstance()->GetMouseMove().z) *
    //                      0.1f});
    //  camera->SetTranslate(camreaTranslate);
    //}
    //if (Input::GetInstance()->GetJoyStick(0, state)) {
    //  // 左スティックの値を取得
    //  float x = (float)state.Gamepad.sThumbLX;
    //  float y = (float)state.Gamepad.sThumbLY;

    //  // 数値が大きいので正規化（-1.0 ～ 1.0）して使うのが一般的
    //  float normalizedX = x / 32767.0f;
    //  float normalizedY = y / 32767.0f;
    //  Vector3 camreaTranslate = camera->GetTranslate();
    //  camreaTranslate = Add(camreaTranslate, Vector3{normalizedX / 60.0f,
    //                                                 normalizedY / 60.0f, 0.0f});
    //  camera->SetTranslate(camreaTranslate);
    //}

    camera->Update();

#ifdef USE_IMGUI
    ImGui::Begin("Debug");

    ImGui::Text("Sprite");
    Vector2 Position = sprite->GetPosition();
    ImGui::SliderFloat2("Position", &(Position.x), 0.1f, 1000.0f);
    sprite->SetPosition(Position);

    ImGui::End();
#endif // USE_IMGUI

    // sprite->SetRotation(sprite->GetRotation() + 0.1f);
    sprite->Update();
}
void TitleScene::Draw() {

    // ParticleManager::GetInstance()->Draw();

    sprite->Draw();
    titleLogoSprite->Draw();
    pressStartSprite->Draw();
    fade_->Draw();

    ///////スプライトの描画
    // sprite->Draw();
}