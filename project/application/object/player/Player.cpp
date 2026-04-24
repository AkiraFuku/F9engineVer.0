#include "Player.h"
#include "Object3D.h"
#include "ModelManager.h"
#include "RailMover.h"
#include "RailPath.h"
#include "InputHandler.h"
#include "imgui.h"
#include "PlayerState.h"
#include "PlayerBehavia.h"
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
    ChangeState(std::make_unique<StateNormal>());
    ChangeBehavior(std::make_unique<BehaviorRoot>());
}

void Player::Update()
{
    HandleInput();
    if (baseState_) baseState_->Update(this);
    if (behavior_) behavior_->Update(this);
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

    ImGui::Separator(); // 区切り線
    ImGui::Text("--- Player States ---");

    // 搭乗ステートの表示
    if (baseState_) {
        ImGui::Text("Base State: %s", baseState_->GetName());
    } else {
        ImGui::Text("Base State: None");
    }

    // ビヘイビア（アクション）ステートの表示
    if (behavior_) {
        ImGui::Text("Behavior: %s", behavior_->GetName());
    } else {
        ImGui::Text("Behavior: None");
    }


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


    if (!rail || !railMover_)
    {
        return;
    }
    railMover_->SetPath(rail);

}
void Player::Move(float ratio)
{
    railMover_->Advance(ratio * (kMoveSpeed_ ));
}
void Player::Jump()
{
    if (isGrounded_) {
        velocity_.y = kJumpAcceleration;
        isGrounded_ = false;
    }
}
void Player::Attack() {
}
float Player::GetRailProgress() const
{

    return railMover_->GetProgress();

}
float Player::GetCurrentDistance() const {
    // RailMoverが持っている現在の走行距離（メートル）を返す
    return railMover_->GetCurrentDistance(); 
}
const RailPath* Player::GetRailPath() const
{
    return railMover_->GetRailPath();
}
void Player::HandleInput()
{
    auto commands = inputHandler_->HandleInput();
    for (auto& command : commands) {
        // 第一段階：搭乗ステートへ
        if (baseState_) {
            baseState_->HandleInput(this, command.get());
        }
    }
}
void Player::ChangeState(std::unique_ptr<IPlayerState> newState) {
    if (baseState_) baseState_->Finalize(this);
    baseState_ = std::move(newState);
    baseState_->Initialize(this);
}
void Player::ChangeBehavior(std::unique_ptr<IPlayerBehavior> newBehavior) {
    behavior_ = std::move(newBehavior);
    behavior_->Initialize(this);
}