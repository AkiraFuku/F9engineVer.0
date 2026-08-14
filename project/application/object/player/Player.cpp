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
#include "GameScene.h"
#include "EffectManager.h"
#include "collider.h"

Player::Player() = default;
Player::~Player() = default;

void Player::Initialize()
{
    inputHandler_ = std::make_unique<InputHandler>();
    object_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("resources/human/", "walk.gltf");
    object_->Initialize();
    //object_->SetModel("player.obj");
    object_->SetModel("walk.gltf");
    animation = std::make_unique<Animation>();

    // animation->Initialize("resources/AnimatedCube", "AnimatedCube.gltf");
    animation->Initialize("resources/human", "walk.gltf");
    animation->SetCurrentTime(0.0f);
    object_->SetAnimations(animation.get());

    railMover_ = std::make_unique<RailMover>();

    // Stateの初期化のみ行い、Behaviorの初期化はState内部で行う
    ChangeState(PlayerStateFactory::CreateState(PlayerFormType::Normal));
    collider_ = std::make_unique<Collider>();
    collider_->initialize(this, Radius);
    object_->Update();
}
void Player::Update()
{
 //   if (!isActive_) return;

    HandleAlive();

    if (!isAlive_) {
        ChangeState(std::make_unique<StateDead>());
    }

    HandleDamage();
    HandleInput();
    RayCastUpdate();
    collider_->Update();

    if (baseState_) baseState_->Update(this);

    UpdateRailPath();
    ImGuiDrawDebugInfo();
}

void Player::Draw()
{
//    if (!isActive_) return;
    object_->Draw();

    collider_->Draw();
}

void Player::SetRailPosition(const Vector2& position)
{
    if (railMover_) {
        // 1. レールの進捗を設定
        railMover_->SetProgress(position.x);

        // 2. 高度(Y)の設定（Y座標の管理変数 worldY_ も同期）
        worldY_ = position.y;

        // 3. 即座にトランスフォーム（位置・回転）を更新
        UpdateRailPath();

        // 4. 地面へのレイキャスト判定と高度補正（シーンが設定済みの場合）
        if (scene_) {
            RayCastUpdate();
            if (isRayHit_) {
                worldY_ = rayHitPoint_.y + kHeightOffset;
                UpdateRailPath(); // 重力補正後の高度で再度トランスフォーム更新
                object_->Update();
            }
        }
    }
}

void Player::AddVelocity(Vector3 v) {
    velocity_ += v;
}

void Player::SetRail(RailPath* rail)
{
    if (!rail || !railMover_) return;
    if (!rail || !railMover_) return;

    // レールパスを適用
    railMover_->SetPath(rail);

    // 物理座標の同期
    UpdateRailPath();

    // 地面へのレイキャスト判定と高度補正（シーンが設定済みの場合）
    if (scene_) {
        RayCastUpdate();
        if (isRayHit_) {
            worldY_ = rayHitPoint_.y + kHeightOffset;
            UpdateRailPath(); // 重力補正後の高度で再度トランスフォーム更新

        }
    }
}

void Player::Move(float ratio)
{
    railMover_->Advance(ratio * (kMoveSpeed_ * deltaTime_));
}

void Player::Jump()
{
    if (isGrounded_) {
        velocity_.y = kJumpAcceleration;
        isGrounded_ = false;
    }
}

void Player::Attack() {}

float Player::GetRailProgress() const {
    return railMover_->GetProgress();
}
float Player::GetCurrentDistance() const {
    return railMover_->GetCurrentDistance();
}
const RailPath* Player::GetRailPath() const {
    return railMover_->GetRailPath();
}

void Player::UpdateRailPath()
{
    Vector3 railPos = railMover_->GetCurrentPosition();
    Vector3 railDir = railMover_->GetCurrentDirection();

    Vector3 finalPos = { railPos.x, worldY_, railPos.z };
    object_->SetTranslate(finalPos);

    float currentFrameAngle = 0.0f;
    if (Length(railDir) > 0.001f) {
        currentFrameAngle = atan2f(railDir.x, railDir.z);
        if (railMover_->GetMoveDirection() == RailMover::MoveDirection::Backward) {
            currentFrameAngle += std::numbers::pi_v<float>;
        }
    }

    object_->SetRotate({ 0.0f, currentFrameAngle, 0.0f });
    object_->Update();
}

void Player::UpdateGravity()
{
    if (isRayHit_ && velocity_.y <= 0.0f) {
        rayHitPalamata_.groundY = rayHitPoint_.y;
        const float kGroundEpsilon = 0.05f;
        float playerBottomY = worldY_ - kHeightOffset;

        if (playerBottomY <= rayHitPalamata_.groundY + kGroundEpsilon) {
            isGrounded_ = true;
        } else {
            isGrounded_ = false;
        }
    } else {
        isGrounded_ = false;
        rayHitPalamata_.groundY = -FLT_MAX;
    }

    if (!isGrounded_) {
        velocity_.y += (kGravity * gravityScale_) * deltaTime_;
    } else {
        if (velocity_.y < 0.0f) {
            velocity_.y = 0.0f;
        }
    }

    worldY_ += velocity_.y * deltaTime_;

    if (isGrounded_ && isRayHit_) {
        worldY_ = rayHitPalamata_.groundY + kHeightOffset;
    }

    if (!isRayHit_ && worldY_ <= rayHitPalamata_.minY + kHeightOffset) {
        worldY_ = rayHitPalamata_.minY + kHeightOffset;
        velocity_.y = 0.0f;
        isGrounded_ = true;
    }
}

void Player::RayCastUpdate()
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
    ray_.origin.y += rayHitPalamata_.rayOffset;
    ray_.diff = { 0.0f, -10.0f - rayHitPalamata_.rayOffset, 0.0f };

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
            if (result == RayTriangleCollisionResult::FrontFace) {
                if (dist < rayHitDistance_) {
                    rayHitDistance_ = dist;
                    rayHitTriangle_ = tri;
                    rayHitPoint_ = tmpHit;
                    isRayHit_ = true;
                }
            }
            result_ = result;
        }
    }

    PrimitiveDrawer::GetInstance()->DrawLine(ray_.origin, Add(ray_.origin, ray_.diff),
        isRayHit_ ? Vector4{ 1,0,0,1 } : Vector4{ 0,1,0,1 });
    if (isRayHit_) {
        PrimitiveDrawer::GetInstance()->DrawSphere({ rayHitPoint_, 0.05f, {} }, { 0,0,1,1 });
    }
}

void Player::HandleInput()
{
    auto commands = inputHandler_->HandleInput();
    for (auto& command : commands) {
        if (baseState_) {
            baseState_->HandleInput(this, command.get());
        }
    }
}

void Player::HandleDamage()
{
    if (hitInvincibilityTimer_ > 0.0f) {
        hitInvincibilityTimer_ -= deltaTime_;
        if (hitInvincibilityTimer_ <= 0.0f) {
            hitInvincibilityTimer_ = 0.0f;
            isDamaged_ = false;
        }
    }
}

void Player::TakeDamage()
{
    if (hitInvincibilityTimer_ <= 0.0f && !isInvincible_) {
        isDamaged_ = true;
        hitInvincibilityTimer_ = kHitInvincibilityDuration_;
        hitPoints_.value--;
    }
}

void Player::HandleAlive()
{
    if (hitPoints_.value <= 0) {
        isAlive_ = false;
    }
}

void Player::ImGuiDrawDebugInfo() {
#ifdef USE_IMGUI
    ImGui::Begin("Debug/Player");
    ImGui::Text("Rail Progress: %.2f", railMover_->GetProgress());
    Vector3 pos = object_->GetTranslate();
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
    ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", velocity_.x, velocity_.y, velocity_.z);

    ImGui::Separator();
    ImGui::Text("--- Player States ---");

    if (baseState_) {
        ImGui::Text("Base State: %s", baseState_->GetName());
    } else {
        ImGui::Text("Base State: None");
    }

    if (baseState_ && baseState_->GetBehavior()) {
        ImGui::Text("Behavior: %s", baseState_->GetBehavior()->GetName());
    } else {
        ImGui::Text("Behavior: None");
    }
    ImGui::Separator();

    if (isDamaged_) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "STATUS: COLLIDING / DAMAGED!");
    } else {
        ImGui::Text("STATUS: Normal");
    }

    ImGui::Text("Hit Points: %d", hitPoints_.value);
    ImGui::ProgressBar(hitInvincibilityTimer_ / kHitInvincibilityDuration_, ImVec2(0, 0), "Hit Timer");

    ImGui::End();
#endif
}

void Player::ChangeState(std::unique_ptr<IPlayerState> newState) {
    if (baseState_) baseState_->Finalize(this);
    baseState_ = std::move(newState);
    if (baseState_) baseState_->Initialize(this);
}

void Player::ChangeBehavior(std::unique_ptr<IPlayerBehavior> newBehavior) {
    if (baseState_) {
        baseState_->ChangeBehavior(this, std::move(newBehavior));
    }
}

void Player::OnCollision(GameObject* other) {
    if (!other || isInvincible_) return;

    if (other->GetCategory() == CollisionCategory::Enemy) {
        Enemy* enemy = dynamic_cast<Enemy*>(other);
        if (!enemy) return;

        const char* playerBehavior = GetBehaviorName();
        const char* playerState = GetStateName();

        if (playerState && strcmp(playerState, "Normal") == 0) {
            const char* enemyState = enemy->GetStateName();

            if (enemyState && strcmp(enemyState, "Normal") == 0) {
                if (hitInvincibilityTimer_ <= 0.0f) {
                    TakeDamage();
                }
            }

            if (playerBehavior && strcmp(playerBehavior, "Attack") == 0) {
                if (enemyState && strcmp(enemyState, "Dead") != 0) {
                    Vector3 now = velocity_;
                    now.x = 0.0f;
                    velocity_ = now;

                    // アタック成功時のBehavior復帰
                    if (baseState_) {
                        baseState_->ChangeBehavior(this, std::make_unique<BehaviorRoot>());
                    }
                }
                return;
            }
        }
    }
}

Vector3 Player::GetDirection() const {
    return railMover_->GetCurrentDirection();
}
int Player::GetMoveDirection() const {
    return int(railMover_->GetMoveDirection());
}

const char* Player::GetStateName() const {
    return baseState_ ? baseState_->GetName() : "NoState";
}

const char* Player::GetBehaviorName() const {
    return (baseState_ && baseState_->GetBehavior()) ? baseState_->GetBehavior()->GetName() : "NoBehavior";
}

void Player::SetScene(Scene* scene) {
    scene_ = scene;
}