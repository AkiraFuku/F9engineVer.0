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




    object_->Update();
}

void Player::Draw()
{
    object_->Draw();
}
void Player::HandleInput()
{
    //　ゲームパッド

    if (!input_->GetInstance()){return;}


    XINPUT_STATE state;
    if (Input::GetInstance()->GetJoyStick(0, state)){
    
        // 1. 入力値の取得と正規化
        float rawX = (float)state.Gamepad.sThumbLX / 32767.0f;
        float rawY = (float)state.Gamepad.sThumbLY / 32767.0f;

        // 2. デッドゾーンの処理
        // スティックの傾きが小さいときは 0 にする
        float deadZone = 0.2f;
        Vector3 moveInput = { rawX, rawY, 0.0f };

        // 入力ベクトルの長さを計算 (自前のVector3Math等があればそれを使用)
        float length = sqrtf(moveInput.x * moveInput.x + moveInput.y * moveInput.y);

        if (length < deadZone) {
            moveInput = { 0.0f, 0.0f, 0.0f };
            float angle = atan2f(moveInput.x, moveInput.y);
            object_->SetRotate({ 0.0f, angle, 0.0f });

        } else {
            // デッドゾーンを超えた場合、入力を正規化して「はみ出し」を抑える
            // かつ、デッドゾーンの境界から最大値までを 0.0 ~ 1.0 に再マップするとより滑らかです
            float factor = (length - deadZone) / (1.0f - deadZone);
            moveInput.x = (moveInput.x / length) * factor;
            moveInput.y = (moveInput.y / length) * factor;
        }

        // 3. 移動速度の適用

        Vector3 currentPos = object_->GetTranslate();

        Vector3 velocity = {
            moveInput.x * kMoveSpeed_,
            moveInput.y * kMoveSpeed_,
            0.0f
        };

      
        object_->SetTranslate(Add(currentPos, velocity));
    

    }
}