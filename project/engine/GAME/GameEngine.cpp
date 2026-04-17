#include "GameEngine.h"
#include "PrimitiveDrawer.h"
#include "LightManager.h"
void GameEngine::Initialize() {

   Framework::Initialize();
  
   
  
   sceneFactory_ = std::make_unique< SceneFactory>();

   
   SceneManager::GetInstance()->SetSceneFactory(sceneFactory_.get());

   
   SceneManager::GetInstance()->ChangeScene("TitleScene");
  SceneManager::GetInstance()->ChangeScene("GameScene");
    PrimitiveDrawer::GetInstance()->Initialize();
};
void GameEngine::Finalize() {  
    SceneManager::GetInstance()->Finalize();
   
    Framework::Finalize();
};
void GameEngine::Update() {
    Framework::Update();
   
    SceneManager::GetInstance()->Update();
    ParticleManager::GetInstance()->Update();
    LightManager::GetInstance()->Update();

 
};
void GameEngine::Draw() {

   
  
    SceneManager::GetInstance()->Draw();

    Framework::Draw();
    ///


}