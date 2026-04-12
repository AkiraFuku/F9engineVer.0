#include "Player.h"
#include "Object3D.h"
#include "ModelManager.h"
#include "RailMover.h"
#include "RailPath.h"
#include "input.h"

Player::Player() = default;
Player::~Player() = default;
void Player::Initialize()
{
    input_ = input_->GetInstance();
    object_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->CreateSphereModel("Sphere", 16);
    object_->Initialize();
    object_->SetModel("Sphere");
    // object_->SetCamera(activeCamera_);
    railMover_ = std::make_unique<RailMover>();
}

void Player::Uppdate()
{
    HandleInput();

    // 重力計算
    if (!isGrounded_) {
        velocity_.y += kGravity;
    }

    // レールの現在位置を取得し、ジャンプの高さを足す
    Vector3 pos = railMover_->GetCurrentPosition();
    pos.y += velocity_.y;

    // 地面判定
    if (pos.y <= 0.0f) {
        pos.y = 0.0f;
        velocity_.y = 0.0f;
        isGrounded_ = true;
    }

    // 回転の設定
    Vector3 dir = railMover_->GetCurrentDirection();
    float angle = atan2f(dir.x, dir.z);
    object_->SetRotate({ 0, angle, 0 });

    object_->SetTranslate(pos);
    object_->Update();
}

void Player::Draw()
{
    object_->Draw();
}
void Player::SetRail(RailPath* rail)
{
    /*    if (rail->GetMaxT() <= 0) { // 初期化を1回にする例
            rail->AddPoint({ 0, 0, 0 });
            rail->AddPoint({ 0, 0, 50 });
            rail->AddPoint({ 50, 0, 100 });
        }*/

    if (!rail||!railMover_)
    {
        return;
    }
    railMover_->SetPath(rail);

}
void Player::HandleInput()
{
    XINPUT_STATE state;
    if (!Input::GetInstance()->GetJoyStick(0, state)) return;

    float rawX = (float)state.Gamepad.sThumbLX / 32767.0f;
    if (std::abs(rawX) > 0.2f) {
        // レール上の移動
        railMover_->Advance(rawX * 0.05f); // 速度は調整
    }

    // ジャンプボタンの判定
    if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) && isGrounded_) {
        velocity_.y = kJumpAcceleration;
        isGrounded_ = false;
    }
}