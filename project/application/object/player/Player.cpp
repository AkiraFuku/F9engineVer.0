#include "Player.h"
#include "Object3D.h"
#include "ModelManager.h"
#include "RailMover.h"
#include "RailPath.h"
#include "InputHandler.h"
#include "imgui.h"
#include "PlayerState.h"
#include "PlayerBehavior.h"
#include "Enemy.h"
#include "Scene.h"
Player::Player() = default;
Player::~Player() = default;
void Player::Initialize()
{
    inputHandler_ = std::make_unique<InputHandler>();
    object_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->CreateSphereModel("Sphere", 16);
    object_->Initialize();
    object_->AddModel("Sphere", "Sphere", "");
    railMover_ = std::make_unique<RailMover>();
    // テスト用：初期ステートをStateNormalからStateRideOnTestに変更
    ChangeState(std::make_unique<StateNormal>());
    ChangeBehavior(std::make_unique<BehaviorRoot>());
}

void Player::Update()
{
    HandleInput();
    if (baseState_) baseState_->Update(this);
    // if (behavior_) behavior_->Update(this);

    UpdateRailPath();
    // --- タイマーの更新 ---
    if (hitVisualTimer_ > 0.0f) {
        hitVisualTimer_ -= (1.0f / 60.0f); // フレームレートに合わせて減算

        // タイマーが 0 以下になったら、フラグをリセットして次の被弾を許可する
        if (hitVisualTimer_ <= 0.0f) {
            hitVisualTimer_ = 0.0f;
            isHit_ = false;
        }
    }

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
    if (baseState_ && baseState_->GetBehavior()) {
        ImGui::Text("Behavior: %s", baseState_->GetBehavior()->GetName());
    } else {
        ImGui::Text("Behavior: None");
    }
    ImGui::Separator(); // 区切り線
    // --- 被弾状態の表示 ---
    if (isHit_) {
        // 赤文字で大きく表示
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "STATUS: COLLIDING / DAMAGED!");
    } else {
        ImGui::Text("STATUS: Normal");
    }

    // タイマーの残りも出しておくと便利
    ImGui::ProgressBar((float)hitVisualTimer_ / kHitVisualDuration, ImVec2(0, 0), "Hit Timer");

    Vector3 dir = railMover_->GetCurrentDirection();
    //進行方向
    ImGui::Text("DIR: (%.2f, %.2f, %.2f)", dir.x, dir.y, dir.z);


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
void Player::AddVelocity(Vector3 v)
{
    velocity_+=v;
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
    railMover_->Advance(ratio * (kMoveSpeed_));
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
void Player::UpdateRailPath()
{


    // 2. レール上の座標を取得 (XZの土台)
    Vector3 railPos = railMover_->GetCurrentPosition();

    // 3. 【重要】レールのXZと、自分のYを合成する
    Vector3 finalPos = { railPos.x, worldY_, railPos.z };

    // 座標と回転の反映
    object_->SetTranslate(finalPos);

    // 進行方向を向く処理
    Vector3 dir = railMover_->GetCurrentDirection();
    float railAngle = atan2f(dir.x, dir.z);

    // ここでは保持している playerAngle_ を優先して適用
    object_->SetRotate({ 0.0f, playerAngle_, 0.0f });

    object_->Update();
}
void Player::UpdateGravity()
{
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
    if (baseState_) {
        baseState_->ChangeBehavior(std::move(newBehavior));
        if (baseState_->GetBehavior()) {
            baseState_->GetBehavior()->Initialize(this);
        }
    }
}
void Player::OnCollision([[maybe_unused]] Enemy* other) {
    // 衝突したらフラグとタイマーをセット
// ただし、クールダウン中なら何もしない
    // プレイヤーの状態を取得
    const char* playerBehavior = GetBehaviorName();
    const char* playerState = GetStateName();

    if (playerState && strcmp(playerState, "Normal") == 0)
    {

        //エネミーの状態を取得
        const char* enemyState = other->GetStateName();
        // 例えば、敵がスタン状態なら当たってもダメージを受けないなどの例外処理

        if (enemyState && strcmp(enemyState, "Normal") == 0)
        {
            if (hitVisualTimer_ <= 0.0f)
            {
                isHit_ = true;
                hitVisualTimer_ = kHitVisualDuration;
                // (必要であれば) ノックバックなどの物理挙動をここに書く



            }
        }

        if (playerBehavior && strcmp(playerBehavior, "Attack") == 0) {

            if (enemyState && strcmp(enemyState, "Dead") != 0)
            {
                Vector3 now=velocity_;
                now.x=0.0f;
                velocity_=now;

                ChangeBehavior(std::make_unique<BehaviorRoot>()); // 通常切り替える

            }
            // 攻撃が当たった場合は、被弾状態にはならない（例外的に無敵）
            return;
        }


    }





}

Vector3 Player::GetDirection() const
{
    return railMover_->GetCurrentDirection();
}

int Player::GetMoveDirection() const
{
    return int(railMover_->GetMoveDirection());
}

const char* Player::GetStateName() const {
    if (baseState_) {
        return baseState_->GetName();
    }
    return "NoState";
}

const char* Player::GetBehaviorName() const {
    if (baseState_ && baseState_->GetBehavior()) {
        return baseState_->GetBehavior()->GetName();
    }
    return "NoBehavior";
}

void Player::SetScene(Scene* scene)
{
    scene_=scene;
}
