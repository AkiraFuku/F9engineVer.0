#include "SceneManager.h"
#include <cassert>
// 静的メンバ変数の実体
std::unique_ptr<SceneManager> SceneManager::instance = nullptr;

SceneManager* SceneManager::GetInstance() {
    if (instance == nullptr) {
        instance.reset(new SceneManager());
        //instance = std::make_unique<SceneManager>();
    }
    return instance.get();
}

void SceneManager::Finalize() {
    if (scene_) {
        scene_->Finalize();
        scene_ = nullptr;
    }
    instance = nullptr;
}

SceneManager::~SceneManager()
{
    if (scene_) {
        scene_->Finalize();
        scene_ = nullptr;
    }
}

void SceneManager::Update() {
    // シーン切り替え処理
    if (nextScene_) {

        if (scene_)
        {
            scene_->Finalize();
            scene_ = nullptr;

        }
        scene_ = std::move(nextScene_);
      

        scene_->SetSceneManager(this);
        scene_->Initialize();

    }

    if (scene_) {
        scene_->Update();
    }
}

void SceneManager::Draw() {
    if (scene_) {
        scene_->Draw();
    }
}

void SceneManager::ChangeScene(const std::string& sceneName)
{
    assert(sceneFactory_);
    assert(nextScene_ == nullptr);
    nextScene_ = sceneFactory_->CreateScene(sceneName);
}
