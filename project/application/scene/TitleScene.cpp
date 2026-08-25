#include "TitleScene.h"
#include "ModelManager.h"
#include "Input.h"
#include "imgui.h"
#include "GameScene.h"
#include "SceneManager.h"
#include "ParticleManager.h"//フレームワークに移植
#include "PSOManager.h"
#include "LightManager.h"
#include "PrimitiveDrawer.h"
#include "Phase.h"
#include "Fade.h"
#include "TitlePhase.h"


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

    ChangePhase(std::make_unique<TitlePhase>());

}
void TitleScene::Finalize() {

}
void TitleScene::Update() {
    if (activeCamera_) {
        activeCamera_->Update();
        activeCamera_->UpdateViewProjection();
    }
    skyBox->Update();
    sprite->Update();
    currentPhase_->Update(this);
}
void TitleScene::Draw() {
    skyBox->Draw();
    sprite->Draw();
}


