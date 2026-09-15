#include "TitleScene.h"
#include "ModelManager.h"
#include "Input.h"
#include "imgui.h"
#include "SceneManager.h"
#include "ParticleManager.h"//フレームワークに移植
#include "PSOManager.h"
#include "LightManager.h"
#include "PrimitiveDrawer.h"
#include "Phase.h"
#include "Fade.h"
#include "TitlePhase.h"
#include "PbdCloth.h"
#include <cmath>


TitleScene::TitleScene() = default;
TitleScene::~TitleScene() = default;

void TitleScene::Initialize() {

    // 1. メインカメラの生成
    camera = std::make_unique<Camera>();
    camera->SetTranslate({ 0.0f, 0.0f, -5.0f });
    cameraMap_["Main"] = std::move(camera);

    // 2. デバッグ用カメラの生成
    auto debugCamera = std::make_unique<Camera>();
    debugCamera->SetTranslate({ 0.0f, 10.0f, -20.0f });
    cameraMap_["Debug"] = std::move(debugCamera);

    // 3. 最初はメインカメラをセット
    ChangeActiveCamera(cameraMap_["Main"].get());


    handle_ = Audio::GetInstance()->LoadAudio("resources/fanfare.mp3");

    // Audio::GetInstance()->PlayAudio(handle_, true);

    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("resources/Title/tekutekuTitle.png");

    LightManager::GetInstance()->AddDirectionalLight({ 0.0f,-1.0f,0.0f }, { 1.0f,1.0f,1.0f }, 1.0f);

    skyBox = std::make_unique<SkyBox>();
    skyBox->Initialize();
    skyBox->SetCamera(activeCamera_);
    skyBox->SetTextureByFilePath("resources/output_skybox.dds");
    Object3dCommon::GetInstance()->SetDefaultSkyBox(skyBox.get());

    Fade::GetInstance()->StartFadeIn(5.0f);


    sprite = std::make_unique<Sprite>();
    sprite->Initialize("resources/Title/tekutekuTitle.png");
    sprite->SetAnchorPoint(Anchor::Center);
    sprite->SetPosition(WinApp::GetInstance()->GetWindowCenter());

    // PBD 布オブジェクトの初期化
    cloth_ = std::make_unique<PbdCloth>();
    // 幅: 4.0 (x: -2.0 ~ 2.0), 高さ: 3.0 (y: 1.5 ~ -1.5), 格子数: 16x16
    cloth_->Initialize({ -2.0f, 1.5f, 0.0f }, { 2.0f, -1.5f, 0.0f }, 16, 16, 0.3f, 1.0f / 60.0f, 0.03f, { 0.0f, -9.8f, 0.0f });
    cloth_->SetTexture("resources/uvChecker.png");
    cloth_->SetCamera(activeCamera_);

    ChangePhase(std::make_unique<TitlePhase>());

    handle_ = Audio::GetInstance()->LoadAudio("resources/Audio/BGM/bgm.mp3");

    BGMHandle_= Audio::GetInstance()->PlayAudio(handle_, true, 0.5f ,"BGM");

}
void TitleScene::Finalize() {
    if (Audio::GetInstance()->IsPlaying(BGMHandle_))
    {
        Audio::GetInstance()->StopAudio(BGMHandle_);
    }

}
void TitleScene::Update() {
    if (activeCamera_) {
        activeCamera_->Update();
        activeCamera_->UpdateViewProjection();
    }
    skyBox->Update();

    // 布の更新（風シミュレーション付き）
    if (cloth_) {
        clothTimer_ += 1.0f / 60.0f;
        if (clothWindEnabled_) {
            // 自然な風の揺れをサイン・コサインの合成波で生成
            float windZ = std::sin(clothTimer_ * 2.5f) * clothWindStrength_ + std::cos(clothTimer_ * 4.7f) * (clothWindStrength_ * 0.4f);
            float windX = std::cos(clothTimer_ * 1.8f) * (clothWindStrength_ * 0.3f);
            cloth_->SetGravity({ windX, -9.8f, windZ });
        } else {
            cloth_->SetGravity({ 0.0f, -9.8f, 0.0f });
        }

        cloth_->SetCamera(activeCamera_);
        cloth_->Update();
    }

    sprite->Update();
    currentPhase_->Update(this);

#ifdef USE_IMGUI
    ImGui::Begin("PBD Cloth Debug");
    if (ImGui::Button("Reset Cloth")) {
        cloth_->Initialize({ -2.0f, 1.5f, 0.0f }, { 2.0f, -1.5f, 0.0f }, 16, 16, 0.3f, 1.0f / 60.0f, 0.03f, { 0.0f, -9.8f, 0.0f });
        cloth_->SetTexture("resources/uvChecker.png");
        cloth_->SetCamera(activeCamera_);
    }
    ImGui::Checkbox("Enable Wind", &clothWindEnabled_);
    ImGui::SliderFloat("Wind Strength", &clothWindStrength_, 0.0f, 20.0f);

    if (cloth_) {
        if (ImGui::Button("Apply Gust Push (+Z)")) {
            auto& solver = cloth_->GetSolver();
            int w = solver.GetWidth();
            int h = solver.GetHeight();
            for (int i = 0; i < w; ++i) {
                for (int j = 1; j < h; ++j) {
                    solver.GetPoint(i, j).velocity.z += 8.0f;
                }
            }
        }
    }
    ImGui::End();
#endif
}
void TitleScene::Draw() {
    skyBox->Draw();
    if (cloth_) {
        cloth_->Draw();
    }
    sprite->Draw();
}


