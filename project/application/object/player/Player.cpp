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
#include "PrimitiveDrawer.h"
#include <numbers>
Player::Player() = default;
Player::~Player() = default;
void Player::Initialize()
{
    inputHandler_ = std::make_unique<InputHandler>();
    object_ = std::make_unique<Object3d>();
    //ModelManager::GetInstance()->CreateSphereModel("Player", 16);
    // ModelManager::GetInstance()->CreatePlaneFromTex("PlayerShadow", "resources/gradationLine.png");
    ModelManager::GetInstance()->LoadModel("resources/player/", "player.obj");
    object_->Initialize();
    object_->SetModel("player.obj");
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
    ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", velocity_.x, velocity_.y, velocity_.z);
    Vector3 rot = object_->GetRotate();
    ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", rot.x, rot.y, rot.z);
    Vector3 scale = object_->GetScale();
    ImGui::Text("Scale: (%.2f, %.2f, %.2f)", scale.x, scale.y, scale.z);

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

    // --- 当たり判定 ---
    Sphere collisionSphere = { object_->GetTranslate(), Radius ,EulerToQuaternion(object_->GetRotate())};
    PrimitiveDrawer::GetInstance()->DrawSphere(collisionSphere, isHit_ ? Vector4{ 1.0f, 0.0f, 0.0f, 1.0f } : Vector4{ 0.0f, 1.0f, 0.0f, 1.0f });
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
    velocity_ += v;
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
    // 1. RailMoverから情報を取得
    Vector3 railPos = railMover_->GetCurrentPosition();
    Vector3 railDir = railMover_->GetCurrentDirection();


    // 2. 座標更新
    Vector3 finalPos = { railPos.x, worldY_, railPos.z };
    object_->SetTranslate(finalPos);

    // 3. 回転の計算（メンバ変数の playerAngle_ を使わずローカルで算出）
    float currentFrameAngle = 0.0f; 
    if (Length(railDir) > 0.001f) {
        // atan2f(x, z) でラジアン角を取得
        currentFrameAngle = atan2f(railDir.x, railDir.z);

        // 進行方向が逆（Backward）なら180度反転
        if (railMover_->GetMoveDirection() == RailMover::MoveDirection::Backward) {
            currentFrameAngle += std::numbers::pi_v<float>;
        }
    }

    // ★重要：SetRotateが「加算」ではなく「上書き」であることを確認してください
    // もしObject3dが内部で角度を蓄積しているなら、ここを 0 にリセットする処理が必要です
    object_->SetRotate({ 0.0f, currentFrameAngle, 0.0f });

    // 4. Cursorの座標更新
  
        // カーソルも「累積」を防ぐため、常に計算した角度の正面に配置
   /*     cursorInstance->transform.translate.x = sinf(currentFrameAngle) * forwardOffset;
        cursorInstance->transform.translate.z = cosf(currentFrameAngle) * forwardOffset;
        cursorInstance->transform.translate.y = heightOffset;*/
   

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
                Vector3 now = velocity_;
                now.x = 0.0f;
                velocity_ = now;

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
    scene_ = scene;
}
