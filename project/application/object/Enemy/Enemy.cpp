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
#include "CameraController.h"
#include "PrimitiveDrawer.h"
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
    RayCastUpdate();
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

                dynamic_cast<GameScene*>(scene_)->TriggerHitStop(0.1f); // 0.1秒のヒットストップ
                dynamic_cast<GameScene*>(scene_)->GetCamera()->RequestShake(0.09f, 3.0f, [](float t) {
                    float inv = 1.0f - t;
                    return inv * inv * inv; // float型を返す
                    }); // 0.1秒のシェイク


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
    if (other->GetCategory() == CollisionCategory::PlayerProjectile)
    {
        isDamaged_ = true;
        PlayHitEffect();
        ChangeState(std::make_unique<StateEnemyDead>());
    }
}

void Enemy::RayCastUpdate()
{
    if (!scene_) return;
    auto gs = dynamic_cast<GameScene*>(scene_);
    if (!gs) return;

    const std::vector<Triangle>& triangles = gs->GetTriangle();
    if (triangles.empty()) {
        isRayHit_ = false;
        rayHitDistance_ = FLT_MAX;
        return;
    }

    ray_.origin = object_->GetTranslate();
    ray_.origin.y += rayHitPalamata_.rayOffset; // 始点を上に持ち上げる

    // 持ち上げた分、レイの長さを伸ばす（あるいは床の下まで届く十分な長さに設定）
    ray_.diff = { 0.0f, -10.0f - rayHitPalamata_.rayOffset, 0.0f };
    // 毎フレーム初期化
    isRayHit_ = false;
    rayHitDistance_ = FLT_MAX;
    result_ = RayTriangleCollisionResult::NoCollision;

    rayHitPoint_ = { 0.0f, 0.0f, 0.0f };
    rayHitTriangle_ = Triangle{};


    for (const auto& tri : triangles) {
        Vector3 tmpHit = {};
        float dist = 0.0f;
        RayTriangleCollisionResult result;

        if (CheckRayTriangle(ray_, tri, &dist, &tmpHit, &result)) {
            // 【重要】表面（FrontFace）に当たったときだけを処理対象にする
            // 裏面（BackFace）は立方体の内側などなので、接地用の床としては無視する
            if (result == RayTriangleCollisionResult::FrontFace) {

                // 表面に当たった中で、最も近い（最も高い位置にある）床を選択
                if (dist < rayHitDistance_) {
                    rayHitDistance_ = dist;
                    rayHitTriangle_ = tri;
                    rayHitPoint_ = tmpHit;

                    isRayHit_ = true; // 表面に最短で当たっているので確実に true
                }
            }
            result_ = result;
        }
    }

    // デバッグ描画
    PrimitiveDrawer::GetInstance()->DrawLine(ray_.origin, Add(ray_.origin, ray_.diff),
        isRayHit_ ? Vector4{ 1,0,0,1 } : Vector4{ 0,1,0,1 });
    if (isRayHit_) {
        PrimitiveDrawer::GetInstance()->DrawSphere({ rayHitPoint_, 0.05f, {} }, { 0,0,1,1 });
    }
}

void Enemy::UpdateGravity()
{
    // レイ判定の結果から接地状態を決める
    if (isRayHit_) {
        rayHitPalamata_.groundY = rayHitPoint_.y;

        const float kGroundEpsilon = 0.05f;
        float playerBottomY = worldY_ - kHeightOffset;

        // 【修正ポイント】上昇中（velocity_.y > 0.0f）は絶対に接地判定にしない
        if (velocity_.y <= 0.0f && playerBottomY <= rayHitPalamata_.groundY + kGroundEpsilon) {
            isGrounded_ = true;
        } else {
            isGrounded_ = false;
        }
    } else {
        // レイが当たっていなければ空中
        isGrounded_ = false;
        rayHitPalamata_.groundY = -FLT_MAX;
    }

    // 重力の適用（空中のときのみ）
    if (!isGrounded_) {
        velocity_.y += (kGravity * gravityScale_) * deltaTime_;
    } else {
        velocity_.y = 0.0f; // 接地しているなら下方向の速度はリセット
    }

    // 位置更新（速度を適用）
    worldY_ += velocity_.y;

    // 地面にめり込んでいたら、床の高さぴったりに補正する
    if (isGrounded_ && isRayHit_) {
        worldY_ = rayHitPalamata_.groundY + kHeightOffset; // 例: 床の上にプレイヤーを配置
    }

    //// レイすら当たらない完全な奈落の場合の最低保証
    if (!isRayHit_ && worldY_ <= rayHitPalamata_.minY + kHeightOffset) {
        worldY_ = rayHitPalamata_.minY + kHeightOffset;
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