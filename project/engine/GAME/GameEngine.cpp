#include "GameEngine.h"
#include "ImGuiManager.h"//フレームワークに移植
#include "SrvManager.h"//フレームワークに移植

void GameEngine::Initialize() {

   Framework::Initialize();
  
   
  
   sceneFactory_ = std::make_unique< SceneFactory>();

   
   SceneManager::GetInstance()->SetSceneFactory(sceneFactory_.get());

   
   SceneManager::GetInstance()->ChangeScene("TitleScene");

};
void GameEngine::Finalize() {  
    SceneManager::GetInstance()->Finalize();
   
    Framework::Finalize();
};
void GameEngine::Update() {
    Framework::Update();
   
    SceneManager::GetInstance()->Update();
    ParticleManager::GetInstance()->Update();
 
};
void GameEngine::Draw() {
    DXCommon::GetInstance()->PreDraw();
    SrvManager::GetInstance()->PreDraw();
   
  
    SceneManager::GetInstance()->Draw();

      ImGuiManager::GetInstance()->End();
    ImGuiManager::GetInstance()->Draw();
    DXCommon::GetInstance()->PostDraw();
    TextureManager::GetInstance()->ReleaseIntermediateResources();

//    Framework::Draw();
    ///


}