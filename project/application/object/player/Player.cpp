#include "Player.h"
#include "Object3D.h"
#include "ModelManager.h"
#include "RailMover.h"
#include "RailPath.h"
#include "InputHandler.h"
#include "imgui.h"

Player::Player() = default;
Player::~Player() = default;
void Player::Initialize()
{
    inputHandler_ = std::make_unique<InputHandler>();
    object_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->CreateSphereModel("Sphere", 16);
    object_->Initialize();
    object_->SetModel("Sphere");
    railMover_ = std::make_unique<RailMover>();
}

void Player::Uppdate()
{
   HandleInput();

    // 1. 重力の計算 (Y軸のみ独立して計算)
    if (!isGrounded_) {
        velocity_.y += kGravity;
    }
    worldY_ += velocity_.y;

    // 地面判定 (Y=0を地面とする場合)
    if (worldY_ <= 0.0f) {
        worldY_ = 0.0f;
        velocity_.y = 0.0f;
        isGrounded_ = true;
    }

    // 2. レール上の座標を取得 (XZの土台)
    Vector3 railPos = railMover_->GetCurrentPosition();

    // 3. 【重要】レールのXZと、自分のYを合成する
    Vector3 finalPos = { railPos.x, worldY_, railPos.z };

    // 座標と回転の反映
    object_->SetTranslate(finalPos);

    // 進行方向を向く処理
    Vector3 dir = railMover_->GetCurrentDirection();
    float angle = atan2f(dir.x, dir.z);
    object_->SetRotate({ 0.0f, angle, 0.0f });

    object_->Update();
}

void Player::Draw()
{
    object_->Draw();

    
#ifdef USE_IMGUI
    ImGui::Begin("Debug/Player");
    // ここにプレイヤーのデバッグ情報を表示
    //レールの進捗を表示
        ImGui::Text("Rail Progress: %.2f", railMover_->GetProgress());
        Vector3 pos = object_->GetTranslate();
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);


    ImGui::End();
#endif // USE_IMGUI
}
void Player::SetRailPosition(const Vector2& position)
 {
        if (railMover_) {
            // レール上の位置を直接設定するための関数
            // 例えば、レールの全長に対して0.0f～1.0fの範囲で位置を指定する場合など
            // ここでは仮にposition.xを進捗として使用する例を示します
            float progress = position.x; // 進捗をx成分から取得（例）
            railMover_->BindProgress(&progress); // 進捗をRailMoverにバインド 
            object_->SetTranslate({ object_->GetTranslate().x, position.y, object_->GetTranslate().z }); // Yは現在のまま、XZはレール上の位置に設定

        }
    }
void Player::SetRail(RailPath* rail)
{


    if (!rail||!railMover_)
    {
        return;
    }
    railMover_->SetPath(rail);

}
void Player::Move(float ratio)
{
    railMover_->Advance(ratio * (kMoveSpeed_ / 60.0f));
}
void Player::Jump()
{
    if (isGrounded_) {
        velocity_.y = kJumpAcceleration;
        isGrounded_ = false;
    }
}
void Player::Attack(){
}
void Player::HandleInput()
{
    // 1. 入力を受け取りコマンドを取得
    auto commands = inputHandler_->HandleInput();
    
    // 2. 全てのコマンドを実行
    for (auto& command : commands) {
        command->Execute(*this);
    }
}