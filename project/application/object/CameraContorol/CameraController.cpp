#include "CameraController.h"
#include "Player.h"  
#include "MathFunction.h"  
#include "Camera.h"
#include "transform.h"
#include <iostream>
#include "RailMover.h"
#include "RailPath.h"
#include "Imgui.h"
#include <cstdlib>
using namespace std;
CameraController::CameraController() = default;
CameraController::~CameraController() = default;
void CameraController::Initialize(Camera* camera) {
    camera_ = camera;
    railMover_ = std::make_unique<RailMover>();

}

void CameraController::SetDrawDistance(float distance) {
    drawDistance_ = distance;
    if (camera_) {
        camera_->SetDrawDistance(distance);
    }
}

void CameraController::Update() {
    if (!camera_)
    {
        return;
    }

#ifdef USE_IMGUI
    ImGui::Begin("Debug/camera");
    if (railMover_) {
        ImGui::Text("Rail Progress: %.2f", railMover_->GetProgress());
    }
    if (target_) {
        Vector3 pos = target_->GetTransform().translate;
        ImGui::Text("Player Pos: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
    }

    // カメラモード切り替え
    const char* modeNames[] = { "AutoOffset (自動計算)", "RailCamera (カメラレール)" };
    int currentModeIdx = (mode_ == CameraMode::AutoOffset) ? 0 : 1;
    if (ImGui::Combo("Camera Mode", &currentModeIdx, modeNames, 2)) {
        mode_ = (currentModeIdx == 0) ? CameraMode::AutoOffset : CameraMode::RailCamera;
    }

    if (mode_ == CameraMode::AutoOffset) {
        ImGui::DragFloat("Offset Distance", &autoDistance_, 0.5f, 5.0f, 100.0f);
        ImGui::DragFloat("Offset Height", &autoHeight_, 0.2f, -20.0f, 50.0f);
        ImGui::DragFloat("LookAt Height", &lookAtHeight_, 0.1f, 0.0f, 10.0f);
        ImGui::DragFloat("Smooth Speed", &smoothSpeed_, 0.01f, 0.01f, 1.0f);
        ImGui::Checkbox("Flip Side", &isFlipSide_);
    }

    if (ImGui::DragFloat("Draw Distance (FarClip)", &drawDistance_, 1.0f, 20.0f, 500.0f)) {
        SetDrawDistance(drawDistance_);
    }
    ImGui::End();
#endif // USE_IMGUI

    if (mode_ == CameraMode::RailCamera && railMover_ && railMover_->isRailSet()) {
        RailCamera();
    } else {
        AutoOffsetCamera();
    }

    RotateCamera();
    // シェイクの更新
    UpdateShake();
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
    camera_->SetTranslate(targetWorldTransform.translate + targetOffset_);
}

void CameraController::RotateCamera() {
    if (!target_ || !camera_) return;

    // プレイヤーの座標
    Vector3 targetPos = target_->GetTransform().translate;
    // 注視点の高さを適用
    targetPos.y += lookAtHeight_;

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
    railMover_->SyncWith(target_->GetRailMover());
    camera_->SetTranslate(railMover_->GetCurrentPosition());
}

void CameraController::AutoOffsetCamera()
{
    if (!target_ || !camera_) return;

    Vector3 playerPos = target_->GetTransform().translate;
    Vector3 railFwd = { 0.0f, 0.0f, 1.0f };

    const RailMover* pMover = target_->GetRailMover();
    if (pMover && pMover->isRailSet()) {
        railFwd = pMover->GetCurrentDirection();
    } else if (stageRail_) {
        float prog = pMover ? pMover->GetProgress() : 0.0f;
        railFwd = stageRail_->GetDirection(prog);
    }

    // 水平進行方向を抽出
    railFwd.y = 0.0f;
    float len = Length(railFwd);
    if (len > 1e-4f) {
        railFwd = railFwd * (1.0f / len);
    } else {
        railFwd = { 0.0f, 0.0f, 1.0f };
    }

    Vector3 up = { 0.0f, 1.0f, 0.0f };
    // レール進行方向に対して右側（外側）ベクトル
    Vector3 right = Cross(up, railFwd);
    float rLen = Length(right);
    if (rLen > 1e-4f) {
        right = right * (1.0f / rLen);
    } else {
        right = { 1.0f, 0.0f, 0.0f };
    }

    if (isFlipSide_) {
        right = right * -1.0f;
    }

    // 目標カメラ位置: プレイヤー位置 + 外側オフセット + 高さオフセット
    Vector3 targetCamPos = playerPos + right * autoDistance_ + up * autoHeight_;

    Vector3 currentCamPos = camera_->GetTranslate();
    if (Length(currentCamPos) < 1e-4f) {
        currentCamPos = targetCamPos;
    }

    Vector3 newCamPos = Lerp(currentCamPos, targetCamPos, smoothSpeed_);
    camera_->SetTranslate(newCamPos);
}


void CameraController::RequestShake(float duration, float power, std::function<float(float)> easingFunc) {
    if (duration <= 0.0f) return;
    
    shakeTimer_ = duration;
    shakeDuration_ = duration; // 最大時間を保存
    shakePower_ = power;
    shakeEasing_ = easingFunc; // イージング関数を登録
}

void CameraController::UpdateShake()
{
    if (shakeTimer_ > 0.0f)
    {
        // 残り時間の割合 (1.0 から始まって 0.0 に近づく)
        float progress = shakeTimer_ / shakeDuration_;

        // イージングを適用して現在のパワーを計算
        float currentPower = shakePower_;
        if (shakeEasing_) {
            currentPower = shakePower_ * shakeEasing_(progress);
        }

        // 減衰したパワーでランダムなズレを計算
        float rx = (((float)std::rand() / RAND_MAX) * 2.0f - 1.0f) * currentPower;
        float ry = (((float)std::rand() / RAND_MAX) * 2.0f - 1.0f) * currentPower;

        shakeOffset_ = { rx, ry, 0.0f };

        // 現在のカメラ座標にシェイクを足す
        Vector3 currentPos = camera_->GetTranslate();
        camera_->SetTranslate(currentPos + shakeOffset_);

        // タイマー更新
        shakeTimer_ -= DXCommon::kDeltaTime;
        if (shakeTimer_ <= 0.0f)
        {
            shakeTimer_ = 0.0f;
            shakeDuration_ = 0.0f;
            shakeOffset_ = { 0.0f, 0.0f, 0.0f };
            shakeEasing_ = nullptr;
        }
    }
    else
    {
        shakeOffset_ = { 0.0f, 0.0f, 0.0f };
    }
}