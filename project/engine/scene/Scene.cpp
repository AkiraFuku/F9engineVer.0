#include "Scene.h"
#include "Object3dCommon.h"
#include "ParticleManager.h"
#include "PrimitiveDrawer.h"
#include "OffScreen.h"
// Sceneクラスに共通関数として作ると便利です
void Scene::ChangeActiveCamera(Camera* targetCamera) {
    activeCamera_ = targetCamera;

    // 各マネージャーに新しいカメラを通知
    Object3dCommon::GetInstance()->SetDefaultCamera(activeCamera_);
    ParticleManager::GetInstance()->SetCamera(activeCamera_);
    PrimitiveDrawer::GetInstance()->SetCamera(activeCamera_);
    OffScreen::GetInstance()->SetCamera(activeCamera_);
}