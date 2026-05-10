#include "CameraController.h"
#include "Player.h"  
#include "MathFunction.h"  
#include "Camera.h"
#include "transform.h"
#include <iostream>
#include "RailMover.h"
#include "RailPath.h"
#include "Imgui.h"
using namespace std;
CameraController::CameraController() = default;
CameraController::~CameraController() = default;
void CameraController::Initialize(Camera* camera) {
    camera_ = camera;
    railMover_ = std::make_unique<RailMover>();

}

void CameraController::Update() {
    if (!camera_)
    {
        return;
    }
    const EulerTransform& targetWorldTransform = target_->GetTransform();


    // カメラをターゲット進行度に合わせて移動
    // 
    //camera_->SetTranslate(targetWorldTransform.translate + targetOffset_);

#ifdef USE_IMGUI
    ImGui::Begin("Debug/camera");
    // ここにプレイヤーのデバッグ情報を表示
    //レールの進捗を表示
    ImGui::Text("Rail Progress: %.2f", railMover_->GetProgress());
    Vector3 pos = target_->GetTransform().translate;
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);




    ImGui::End();
#endif // USE_IMGUI


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
    if (!target_ || !camera_) return;

    // プレイヤーの座標
    Vector3 targetPos = target_->GetTransform().translate;
    // 注視点を少し上げる（足元ではなく腰や頭あたりにする）
    targetPos.y += 2.0f;

    // LookAt関数の実装（ターゲットの方向を向くように回転角を計算）
    Vector3 cameraPos = camera_->GetTranslate();
    Vector3 direction = Normalize(targetPos - cameraPos);

    float angleY = std::atan2(direction.x, direction.z);
    float distanceXZ = std::sqrt(direction.x * direction.x + direction.z * direction.z);
    float angleX = std::atan2(-direction.y, distanceXZ);

    camera_->SetRotate({ angleX, angleY, 0.0f });
}

void CameraController::RailCamera()
{
 if (!target_ || !railMover_ || !railMover_->isRailSet()) return;

    // 1. プレイヤー側のレール情報を取得
    const RailPath* playerPath = target_->GetRailPath(); // PlayerにGetRailPath()を追加してください
    if (!playerPath) return;

    // 2. プレイヤーが「全行程の何％」にいるかを算出 (0.0f ～ 1.0f)
    float playerDist = target_->GetCurrentDistance();
    float playerTotalLen = playerPath->GetTotalLength();
    float progressRate = playerDist / playerTotalLen;

    // 3. カメラ側のレールの「同じ％」にあたる距離を算出
    const RailPath* cameraPath = railMover_->GetRailPath();
    float cameraTotalLen = cameraPath->GetTotalLength();
    float targetCameraDist = cameraTotalLen * progressRate;

    // 4. その距離に対応する T を取得して適用
    float cameraT = cameraPath->GetTFromDistance(targetCameraDist);
    railMover_->SetProgress(cameraT);

    // 座標更新
    camera_->SetTranslate(railMover_->GetCurrentPosition());
}
