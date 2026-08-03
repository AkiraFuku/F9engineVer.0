#include "GameEngine.h"
#include "PrimitiveDrawer.h"
#include "OffScreen.h"
void GameEngine::Initialize() {

    Framework::Initialize();



    sceneFactory_ = std::make_unique< SceneFactory>();


    SceneManager::GetInstance()->SetSceneFactory(sceneFactory_.get());


    SceneManager::GetInstance()->ChangeScene("GameScene");
    PrimitiveDrawer::GetInstance()->Initialize();
    OffScreen::GetInstance()->SetEffectFlags(
        PostEffectFlag::DepthOutline
    );
};
void GameEngine::Finalize() {
    SceneManager::GetInstance()->Finalize();

    Framework::Finalize();
};
void GameEngine::Update() {
    Framework::Update();

    SceneManager::GetInstance()->Update();
    
}
void GameEngine::PreDraw()
{
    Framework::PreDraw();
}
;
void GameEngine::Draw() {

    SceneManager::GetInstance()->Draw();

    Framework::Draw();
    ///


}