#include "Scene.h"
#include "Object3dCommon.h"
#include "ParticleManager.h"
#include "PrimitiveDrawer.h"
// Sceneクラスに共通関数として作ると便利です
void Scene::ChangeActiveCamera(Camera* targetCamera) {
    activeCamera_ = targetCamera;

    // 各マネージャーに新しいカメラを通知
    Object3dCommon::GetInstance()->SetDefaultCamera(activeCamera_);
    ParticleManager::GetInstance()->Setcamera(activeCamera_);
    PrimitiveDrawer::GetInstance()->SetCamera(activeCamera_);
}