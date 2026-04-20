#include "CameraController.h"
#include <algorithm>  
#include "CameraController.h"  
#include "Player.h"  
#include "MathFunction.h"  
#include "Camera.h"
#include "transform.h"
#include <iostream>
#include "RailMover.h"
#include "RailPath.h"
using namespace std;
CameraController::CameraController() = default;
CameraController::~CameraController() = default;
void CameraController::Initialize(Camera* camera) {
    camera_ = camera;
    railMover_ = std::make_unique<RailMover>();

}

void CameraController::Update() {
    const EulerTransform& targetWorldTransform = target_->GetTransform();


    // カメラをターゲット進行度に合わせて移動
    // 
    //camera_->SetTranslate(targetWorldTransform.translate + targetOffset_);



    RailCamera();

    RotateCamera();

    camera_->Update();
}

void CameraController::SetRailPath(const RailPath* path)
{
    if (railMover_) {
        railMover_->SetPath(path);
    }
}

void CameraController::SetRailProgress(float progress)
{
    if (railMover_)
    {
        railMover_->BindProgress(&progress);

    }
}

void CameraController::Reset() {
    const EulerTransform& targetWorldTransform = target_->GetTransform();
    // 必要に応じて targetWorldTransform を使用して処理を追加  
    //camera_->translation_ = Add(targetWorldTransform.translation_, targetOffset_);
    camera_->SetTranslate(targetWorldTransform.translate + targetOffset_);

}

void CameraController::RequestShake(float duration, float power) {
    shakeTimer_ = duration;
    shakePower_ = power;
}

void CameraController::RotateCamera() {
    if (!target_)return;
    Vector3 targetPos = target_->GetTransform().translate;
    Vector3 cameraPos = camera_->GetTranslate();

    // 方向ベクトル (Vector3 の引き算)
    Vector3 direction = {
        targetPos.x - cameraPos.x,
        targetPos.y - cameraPos.y,
        targetPos.z - cameraPos.z
    };

    // 2. 回転角を計算
    // Y軸周りの回転 (左右): XとZの差分から計算
    float angleY = std::atan2(direction.x, direction.z);

    // X軸周りの回転 (上下): 水平距離と高さの差分から計算
    float distanceXZ = std::sqrt(direction.x * direction.x + direction.z * direction.z);
    float angleX = std::atan2(-direction.y, distanceXZ); // - をつけることで対象を見下ろす/見上げる

    // 3. カメラに回転を適用
    camera_->SetRotate({ angleX, angleY, 0.0f });
}

void CameraController::RailCamera()
{
    if (railMover_->isRailSet() && target_)
    {
        float progress = target_->GetRailProgress();    
        railMover_->Advance(progress - railMover_->GetProgress());
        Vector3 railPos = railMover_->GetCurrentPosition();
        Vector3 desiredPos = Add(railPos, targetOffset_);
        camera_->SetTranslate(desiredPos);
    }
}
