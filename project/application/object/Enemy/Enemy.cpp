#include "Enemy.h"
#include "Camera.h"
#include "Object3D.h"
#include "ModelManager.h"
#include "RailMover.h"
#include "RailPath.h"
#include <cmath>
#include "enemyBehavior.h"
#include "EnemyState.h"
#include "Player.h"
#include "imgui.h"
#include "Robot.h"
#include "ParticleEmitter.h"
#include "GameScene.h"
Enemy::Enemy() = default;
Enemy::~Enemy() = default;
void Enemy::SetRobot(std::unique_ptr<Robot> robot) {
    robot_ = std::move(robot);
}
void Enemy::Initialize()
{
    object_ = std::make_unique<Object3d>();
    // エネミー用にCubeモデルを作成（既存ならそれを使用）
    ModelManager::GetInstance()->CreateSphereModel("Enemy");

    object_->Initialize();
    object_->SetModel("Enemy");

    railMover_ = std::make_unique<RailMover>();
    ChangeBehavior(std::make_unique<EnemyBehaviorPatrol>());
    ChangeState(std::make_unique<StateEnemyNormal>());
    // セット名 "GameEffects"、グループ名は空（セット内のすべてのエフェクトを発生させる）
    hitParticle_ = std::make_unique<ParticleEmitter>("GameEffects", "", object_->GetTransform(), 3, 10.0f, 0.0f);

}

void Enemy::Update()
{

    if (hitInvincibilityTimer_ > 0.0f) {
        hitInvincibilityTimer_ -= (1.0f / 60.0f); // 60FPSを想定した減算

        if (hitInvincibilityTimer_ <= 0.0f) {
            hitInvincibilityTimer_ = 0.0f;
            isDamaged_ = false; // クールダウン終了
        }
    }

    // 1. 現在の状態を更新
    if (state_) {
        state_->Update(this);
    }

    // 3. 物理計算とレール座標の更新
    UpdatePhysics();
}

void Enemy::Draw()
{
    if (object_) {
        object_->Draw();
    }

#ifdef USE_IMGUI
    ImGui::Begin("Debug/Enemy");

    // 敵の基本情報
    if (object_) {
        Vector3 pos = object_->GetTranslate();
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
    }

    // レールの進捗
    if (railMover_) {
        ImGui::Text("Rail Progress: %.2f", railMover_->GetProgress());
        Vector3 dir = railMover_->GetCurrentDirection();
        ImGui::Text("Direction: (%.2f, %.2f, %.2f)", dir.x, dir.y, dir.z);
    }

    ImGui::Separator();
    ImGui::Text("--- Enemy States ---");

    // 敵の状態表示
    if (state_) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "State: %s", state_->GetName());
    } else {
        ImGui::Text("State: None");
    }

    // 敵の行動表示
    if (behavior_) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Behavior: %s", behavior_->GetName());
    } else {
        ImGui::Text("Behavior: None");
    }

    ImGui::Separator();

    // 物理情報
    ImGui::Text("IsGrounded: %s", isGrounded_ ? "True" : "False");
    ImGui::Text("WorldY: %.2f", worldY_);
    ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", velocity_.x, velocity_.y, velocity_.z);

    ImGui::Separator();
    if (isDamaged_) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Status: CoolDown (Hit!)");
    } else {
        ImGui::Text("Status: Ready");
    }
    ImGui::ProgressBar(hitInvincibilityTimer_ / kHitInvincibilityDuration_, ImVec2(0, 0), "CD Timer");

    ImGui::End();
#endif // USE_IMGUI
}

void Enemy::SetRailPosition(const Vector2& position)
{
    if (railMover_) {
        // レール上の位置を直接設定するための関数
        // 例えば、レールの全長に対して0.0f～1.0fの範囲で位置を指定する場合など
        // ここでは仮にposition.xを進捗として使用する例を示します
        // 進捗をx成分から取得（例）
        railMover_->SetProgress(position.x); // 進捗をRailMoverにバインド 
        object_->SetTranslate({ object_->GetTranslate().x, position.y, object_->GetTranslate().z }); // Yは現在のまま、XZはレール上の位置に設定

    }
}

void Enemy::SetRail(RailPath* rail)
{
    if (rail && railMover_) {
        railMover_->SetPath(rail);
    }
}

void Enemy::Move(float ratio)
{
    if (railMover_) {
        // 毎フレームの移動量を計算して進める
        railMover_->Advance(ratio * (kMoveSpeed_));
    }
}
void Enemy::ChangeBehavior(std::unique_ptr<IEnemyBehavior> newBehavior) {
    if (behavior_) behavior_->Finalize(this);
    behavior_ = std::move(newBehavior);
    if (behavior_) behavior_->Initialize(this);
}

void Enemy::ChangeState(std::unique_ptr<IEnemyState> newState) {
    if (state_) state_->Finalize(this);
    state_ = std::move(newState);
    if (state_) state_->Initialize(this);
}

void Enemy::UpdatePhysics() {
    // Player::UpdateRailPath() と同様のロジック


    Vector3 railPos = railMover_->GetCurrentPosition();
    Vector3 finalPos = { railPos.x, worldY_, railPos.z };
    object_->SetTranslate(finalPos);

    // 回転処理
    Vector3 dir = railMover_->GetCurrentDirection();
    float angle = atan2f(dir.x, dir.z);
    object_->SetRotate({ 0.0f, angle, 0.0f });

    object_->Update();
}
// Enemy.cpp
void Enemy::OnCollision(ICollider* other) {

    // ぶつかった相手がPlayerかどうかを確認
    if (!other || isDamaged_ || IsDead()) return;

    if (other->GetCategory() == CollisionCategory::Player) {
        Player* player = dynamic_cast<Player*>(other);

        const char* playerBehavior = player->GetBehaviorName();
        const char* playerState = player->GetStateName();

        // プレイヤーが通常状態で攻撃中かチェック
        if (playerState && strcmp(playerState, "Normal") == 0) {
            //攻撃中ならエネミーの状態遷移
            if (playerBehavior && strcmp(playerBehavior, "Attack") == 0) {

                //　ゲームシーンを持っているならヒットストップを起こす

                dynamic_cast<GameScene*>(scene_)->TriggerHitStop(); // 0.1秒のヒットストップ

                PlayHitEffect();
                isDamaged_ = true;
                hitInvincibilityTimer_ = kHitInvincibilityDuration_;
                    // ★ここがポイント：ロボットを持っていればプレイヤーを変身させる
                    if (robot_ && robot_->CreateRideOnState()) {
                        // Robotが持っているステートをクローン、あるいは特定のステートを生成して渡す
                        // 現在の設計なら、StateRideOnTestなどの具体的な型をRobot側で定義しておく
                        player->ChangeState(robot_->CreateRideOnState());
                    }

                    ChangeState(std::make_unique<StateEnemyDead>());
               
            }
        }
    }
    //　弾カテゴリの判定
    if (other->GetCategory()==CollisionCategory::PlayerProjectile)
    {
         isDamaged_ = true;
    PlayHitEffect();
    ChangeState(std::make_unique<StateEnemyDead>());
    }
}

void Enemy::UpdateGravity()
{
    if (!isGrounded_) {
        velocity_.y += kGravity;
    }
    worldY_ += velocity_.y;

    if (worldY_ <= 0.0f) {
        worldY_ = 0.0f;
        velocity_.y = 0.0f;
        isGrounded_ = true;
    }
}

const char* Enemy::GetStateName() const
{
    return state_ ? state_->GetName() : "None";
}

const char* Enemy::GetBehaviorName() const
{
    return behavior_ ? behavior_->GetName() : "None";
}

bool Enemy::IsDead() const
{
    return state_ && strcmp(state_->GetName(), "Dead") == 0;
}

void Enemy::PlayHitEffect() {
    if (hitParticle_) {

        hitParticle_->SetTransform(object_->GetTransform());
        hitParticle_->Emit();
    }
}