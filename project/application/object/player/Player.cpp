#include "Player.h"
#include "Object3D.h"
#include "ModelManager.h"
#include "input.h"
void Player::Initialize()
{
    input_ = input_->GetInstance();
    object_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->CreateSphereModel("Sphere", 16);
    object_->Initialize();
    object_->SetModel("Sphere");
    // object_->SetCamera(activeCamera_);

}

void Player::Uppdate()
{
    HandleInput();

    // --- 物理計算（重力の適用） ---
    if (!isGrounded_) {
        velocity_.y += kGravity; // 毎フレーム重力を加算（落下）
    }

    // 座標の更新
    Vector3 currentPos = object_->GetTranslate();
    currentPos.x += velocity_.x;
    currentPos.y += velocity_.y;
    currentPos.z += velocity_.z;

    // --- 地面との判定（簡易版） ---
    // Y=0 を地面と仮定
    if (currentPos.y <= 0.0f) {
        currentPos.y = 0.0f;
        velocity_.y = 0.0f;
        isGrounded_ = true;
    }

    object_->SetTranslate(currentPos);
    object_->Update();
}

void Player::Draw()
{
    object_->Draw();
}
void Player::HandleInput()
{
    if (!input_->GetInstance()) return;

    XINPUT_STATE state;
    if (Input::GetInstance()->GetJoyStick(0, state)) {
        // --- 1. 左右移動の処理 ---
        float rawX = (float)state.Gamepad.sThumbLX / 32767.0f;
        float rawY = (float)state.Gamepad.sThumbLY / 32767.0f;
        float deadZone = 0.2f;

        Input::GetInstance()->SetDeadZone(0, static_cast<int>(deadZone * 32767), static_cast<int>(deadZone * 32767));
        Vector3 moveInput = { rawX, rawY, 0.0f };
        float length = sqrtf(moveInput.x * moveInput.x + moveInput.y * moveInput.y);

        if (length > deadZone) {
            float factor = (length - deadZone) / (1.0f - deadZone);
            // XZ平面の移動速度をセット
            velocity_.x = (moveInput.x / length) * factor * kMoveSpeed_;
            velocity_.z = (moveInput.y / length) * factor * kMoveSpeed_;

            // 回転（移動方向を向く）
            float angle = atan2f(moveInput.x, moveInput.y);
            object_->SetRotate({ 0.0f, angle, 0.0f });
        } else {
            // スティックを離したら水平移動は止める
            velocity_.x = 0.0f;
            velocity_.z = 0.0f;
        }

        // --- 2. ジャンプの入力処理 ---
        // Aボタンが押された瞬間に、接地していれば跳ねる
        if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) && isGrounded_) {
            velocity_.y = kJumpAcceleration;
            isGrounded_ = false; // 空中へ
        }
    }
}