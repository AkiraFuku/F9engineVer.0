#include "Enemy.h"
#include "Camera.h"
#include "Object3D.h"
#include "ModelManager.h"
#include "RailMover.h"
#include "RailPath.h"
#include <cmath>
#include "enemyBehavior.h"
#include "EnemyAction.h"
#include "EnemyState.h"
#include "Player.h"
#include "imgui.h"
#include "Robot.h"
#include "ParticleEmitter.h"
#include "GameScene.h"
#include "CameraController.h"
#include "PrimitiveDrawer.h"
#include "Collider.h"
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
    ModelManager::GetInstance()->LoadModel("resources/taru", "taru.obj");

    object_->Initialize();
    object_->SetModel("taru.obj");
    object_->SetScale({ 0.9f,0.9f,0.9f });
    object_->SetRotate({ 90.0f,0.0f,0.0f });


    railMover_ = std::make_unique<RailMover>();
    ChangeBehavior(std::make_unique<EnemyBehaviorPatrol>());
    ChangeState(std::make_unique<StateEnemyNormal>());
    // セット名 "GameEffects"、グループ名は空（セット内のすべてのエフェクトを発生させる）
    hitParticle_ = std::make_unique<ParticleEmitter>("GameEffects", "", object_->GetTransform(), 3, 10.0f, 0.0f);

    collider_ = std::make_unique<Collider>();
    collider_->initialize(this, radius_);

    object_->Update();

}

void Enemy::Update()
{

    if (hitInvincibilityTimer_ > 0.0f) {
        hitInvincibilityTimer_ -= deltaTime_; // 60FPSを想定した減算

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

    collider_->Update();

}

void Enemy::UpdateTransform()
{
    RayCastUpdate();
    if (isRayHit_) {
        worldY_ = rayHitPoint_.y + kHeightOffset;
    }
    UpdatePhysics();
    if (collider_) {
        collider_->Update();
    }
}

void Enemy::Draw()
{
    if (object_) {
        object_->Draw();
    }
    collider_->Draw();

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
        railMover_->SetProgress(position.x);
        worldY_ = position.y;

        // 2. 物理座標の同期
        UpdatePhysics();

        // 3. 地面へのレイキャスト判定と高度補正（※シーンが設定済みの場合）
        if (scene_) {
            RayCastUpdate();
            if (isRayHit_) {
                worldY_ = rayHitPoint_.y + kHeightOffset;
                UpdatePhysics(); // 重力補正後の高度で再度トランスフォーム更新
                object_->Update();
            }
        }

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
        railMover_->Advance(ratio * (kMoveSpeed_ * deltaTime_));
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
    object_->SetRotate(initialRotationOffset_ + Vector3{ 0.0f, angle, 0.0f });
    object_->Update();
}
// Enemy.cpp
void Enemy::OnCollision(GameObject* other) {

    // ぶつかった相手がPlayerかどうかを確認
    if (!other || isDamaged_ || IsDead()) return;

    if (other->GetCategory() == CollisionCategory::Player) {
        Player* player = dynamic_cast<Player*>(other);
        if (!player) {
            return;
        }

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
                if (robot_) {
                    // 1. ロボットからプレイヤー用 Factory を取得
                    auto factory = robot_->CreatePlayerFactory();

                    if (factory) {
                        // 2. Factory を使って State を生成 (Factoryが自動で State に自身をセットしてくれる)
                        auto rideOnState = factory->CreateState();

                        // 3. プレイヤーの State を切り替える
                        player->ChangeState(std::move(rideOnState));
                    }
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

    // 始点を敵の位置から少し高めに持ち上げる（めり込み時でも確実に地面の上から下向きに照射）
    ray_.origin = object_->GetTranslate();
    ray_.origin.y += 2.0f;

    // 下方向へ十分な長さのレイを飛ばす
    ray_.diff = { 0.0f, -25.0f, 0.0f };

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
            // FrontFace と BackFace の両方を対象にする（左手系・モデルの巻き順差異への対応）
            if (result == RayTriangleCollisionResult::FrontFace || result == RayTriangleCollisionResult::BackFace) {

                // 面法線を計算して上向き（床面）であることを確認（垂直な壁や急崖は除外）
                Vector3 v01 = Subtract(tri.vertices[1], tri.vertices[0]);
                Vector3 v12 = Subtract(tri.vertices[2], tri.vertices[1]);
                Vector3 normal = Cross(v01, v12);
                float nLen = Length(normal);
                if (nLen > 0.0001f) {
                    normal = Normalize(normal);
                    if (result == RayTriangleCollisionResult::BackFace) {
                        normal = Multiply(-1.0f, normal);
                    }
                    if (normal.y < 0.2f) {
                        continue; // 壁や天井面は床判定から除外
                    }
                }

                // 最も近い（最も高い位置にある）床を選択
                if (dist < rayHitDistance_) {
                    rayHitDistance_ = dist;
                    rayHitTriangle_ = tri;
                    rayHitPoint_ = tmpHit;
                    result_ = result;
                    isRayHit_ = true;
                }
            }
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
    if (isRayHit_) {
        rayHitPalamata_.groundY = rayHitPoint_.y;
        float targetY = rayHitPalamata_.groundY + kHeightOffset;
        float enemyBottomY = worldY_ - kHeightOffset;

        // 1. 上昇中（ジャンプ中 velocity_.y > 0.0f）の処理
        if (velocity_.y > 0.0f) {
            isGrounded_ = false;
            velocity_.y += (kGravity * gravityScale_) * deltaTime_;
            worldY_ += velocity_.y * deltaTime_;
        }
        else {
            // 2. 下降中または静止中（velocity_.y <= 0.0f）の着地・吸着処理
            // 地面以下に達した場合、または接地中の下り坂吸着範囲内にある場合
            if (worldY_ <= targetY + 0.05f || (isGrounded_ && enemyBottomY <= rayHitPalamata_.groundY + 0.5f)) {
                isGrounded_ = true;
                worldY_ = targetY; // 地面に確実にスナップ
                velocity_.y = 0.0f;
            } else {
                // 空中から落下中の場合
                isGrounded_ = false;
                velocity_.y += (kGravity * gravityScale_) * deltaTime_;
                worldY_ += velocity_.y * deltaTime_;

                // 落下した結果、地面を突き抜けた場合は地面で止める
                if (worldY_ <= targetY) {
                    worldY_ = targetY;
                    velocity_.y = 0.0f;
                    isGrounded_ = true;
                }
            }
        }
    } else {
        // 地面が検出されない場合（落下中または奈落）
        isGrounded_ = false;
        velocity_.y += (kGravity * gravityScale_) * deltaTime_;
        worldY_ += velocity_.y * deltaTime_;

        // 奈落の最低保証
        if (worldY_ <= rayHitPalamata_.minY + kHeightOffset) {
            worldY_ = rayHitPalamata_.minY + kHeightOffset;
            velocity_.y = 0.0f;
            isGrounded_ = true;
        }
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

float Enemy::GetCurrentDistance() const {
    return railMover_ ? railMover_->GetCurrentDistance() : 0.0f;
}

const RailMover* Enemy::GetRailMover() const {
    return railMover_.get();
}