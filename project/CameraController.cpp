#include "CameraController.h"
#include <algorithm>  
#include "CameraController.h"  
#include "Player.h"  
#include "MathFunction.h"  
#include "Camera.h"
#include "transform.h"
#include <iostream>

using namespace std;
void CameraController::Initialize(Camera* camera) {
    camera_ = camera;
}

void CameraController::Update() {
    const EulerTransform& targetWorldTransform = target_->GetTransform();

/*    if (isClearPhase_) {
        // --- クリアフェーズ：プレイヤーを中央に捉えてズーム ---

        // 1. 目標地点は「プレイヤーの座標 + クリア用オフセット」
        // 先読み（velocity）は入れないことで中央に固定する
        desetination_ = targetWorldTransform.translate + clearOffset_;

        // 2. 線形補間(Lerp)で滑らかに移動させる
        // 0.1f は追従速度。お好みで調整してください


        camera_->SetTranslate(Lerp(camera_->GetTranslate(), desetination_, 0.1f));
    } else {



        desetination_ = targetWorldTransform.translate + targetOffset_ + target_->GetVelocity() * kVelocityBias;
        camera_->SetTranslate(Lerp(camera_->GetTranslate(), desetination_, 0.1f)); // 緩やかに追従するように補間
        if (shakeTimer_ > 0.0f) {
            // 【変更点】乱数(rand)ではなく、sin波を使ってゆっくり揺らす
            // 係数(20.0fなど)を小さくすると、もっとゆっくりになります
            float frequency = 5.5f; // 揺れの速さ（周波数）

            // 時間経過で滑らかに変化する値を作成
            float offsetX = std::sin(shakeTimer_ * frequency) * shakePower_;
            float offsetY = std::cos(shakeTimer_ * frequency) * shakePower_;

            // カメラ座標に加算
            camera_->SetTranslate(camera_->GetTranslate() + Vector3(offsetX, offsetY, 0.0f));


            // タイマーを減らす
            shakeTimer_ -= 1.0f / 60.0f; // 120.0fだと減りが遅いので、60fps基準なら60.0fが自然です
        }

        //Move move={{desetination_.x+targetMargin_.left,desetination_.y+targetMargin_.bottom},{desetination_.x+targetMargin_.right,desetination_.y+targetMargin_.top}};

        camera_->SetTranslate(Vector3(
            max(camera_->GetTranslate().x, desetination_.x + targetMargin_.left),
            max(camera_->GetTranslate().y, desetination_.y + targetMargin_.bottom),
            camera_->GetTranslate().z
        ));
        camera_->SetTranslate(Vector3(
            min(camera_->GetTranslate().x, desetination_.x + targetMargin_.right),
            min(camera_->GetTranslate().y, desetination_.y + targetMargin_.top),
            camera_->GetTranslate().z
        ));
        camera_->SetTranslate(Vector3(
            camera_->GetTranslate().x,
            min(camera_->GetTranslate().y, desetination_.y + targetMargin_.top),
            camera_->GetTranslate().z
        ));

        //	camera_->translation_.x =clamp(camera_->translation_.x,moveArea_.right, moveArea_.left);
        //	camera_->translation_.y =clamp(camera_->translation_.y,moveArea_.bottom, moveArea_.top);
            // 修正: std::max と std::min を使用するために std:: を明示的に指定  
        camera_->SetTranslate(Vector3(
            max(camera_->GetTranslate().x, moveArea_.left),
            max(camera_->GetTranslate().y, moveArea_.top),
            camera_->GetTranslate().z
        )); 
        camera_->SetTranslate(Vector3(
            min(camera_->GetTranslate().x, moveArea_.right),
            min(camera_->GetTranslate().y, moveArea_.bottom),
            camera_->GetTranslate().z
        ));
        camera_->SetTranslate(Vector3(
            camera_->GetTranslate().x,
            max(camera_->GetTranslate().y, moveArea_.top),
            camera_->GetTranslate().z
        ));
    }*/


    RotateCamera();

    camera_->Update();
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