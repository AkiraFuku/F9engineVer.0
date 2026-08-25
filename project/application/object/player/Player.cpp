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

    animation->Initialize("resources/human", "walk.gltf");
    animation->SetCurrentTime(0.0f);
    object_->SetAnimations(animation.get());

    railMover_ = std::make_unique<RailMover>();

    // Stateの初期化のみ行い、Behaviorの初期化はState内部で行う
    ChangeState(PlayerStateFactory::CreateState(PlayerFormType::Normal));

    InitializeRays();

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
    HandleKnockback();
    HandleInput();
    RayCastUpdate();
    collider_->Update();

    if (baseState_) baseState_->Update(this);

    UpdateRailPath();
    ImGuiDrawDebugInfo();
}

void Player::UpdateTransform()
{
    RayCastUpdate();
    UpdateRailPath();
    if (collider_) {
        collider_->Update();
    }
}

void Player::Draw()
{
    //    if (!isActive_) return;

        // 無敵時間中のモデル点滅（0.08秒ごとに表示/非表示を切り替え）
    if (hitInvincibilityTimer_ > 0.0f) {
        const float kBlinkInterval = 0.08f;
        if (fmodf(hitInvincibilityTimer_, kBlinkInterval * 2.0f) < kBlinkInterval) {
            if (collider_) {
                collider_->Draw();
            }
            return;
        }
    }

    object_->Draw();

    if (collider_) {
        collider_->Draw();
    }
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
    finalPos = Add(finalPos, wallPushOffset_);
    object_->SetTranslate(finalPos);

    // ノックバック中は向きを変更せず直前の向きを固定
    if (!isKnockback_) {
        float currentFrameAngle = 0.0f;
        if (Length(railDir) > 0.001f) {
            currentFrameAngle = atan2f(railDir.x, railDir.z);
            if (railMover_->GetMoveDirection() == RailMover::MoveDirection::Backward) {
                currentFrameAngle += std::numbers::pi_v<float>;
            }
        }
        currentAngle_ = currentFrameAngle;
    }

    object_->SetRotate({ 0.0f, currentAngle_, 0.0f });
    object_->Update();
}

void Player::CheckGroundCollision()
{
    auto floorRay = GetRayInfo("Floor");
    bool hitFloor = floorRay && floorRay->isColide;

    if (hitFloor && velocity_.y <= 0.0f) {
        rayHitPalamata_.groundY = floorRay->crossPoint.y;
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

    // めり込み補正（接地時に地面の高さに合わせる）
    if (isGrounded_ && hitFloor) {
        worldY_ = rayHitPalamata_.groundY + kHeightOffset;
    }

    // 奈落の最低保証（落下・死の防止処理：既存のコードを維持）
    if (!hitFloor && worldY_ <= rayHitPalamata_.minY + kHeightOffset) {
        worldY_ = rayHitPalamata_.minY + kHeightOffset;
        velocity_.y = 0.0f;
        isGrounded_ = true;
    }
}

void Player::UpdateGravity()
{
    // 1. 地面の当たり判定
    CheckGroundCollision();

    // 2. 重力加速度の適用
    if (!isGrounded_) {
        velocity_.y += (kGravity * gravityScale_) * deltaTime_;
    } else {
        if (velocity_.y < 0.0f) {
            velocity_.y = 0.0f;
        }
    }

    // 3. 速度による位置更新
    worldY_ += velocity_.y * deltaTime_;
}

void Player::InitializeRays() {
    rayList_.clear();

    CollisionRayInfo floorRay;
    floorRay.name = "Floor";
    // 実際のoriginやdiffは毎フレーム更新
    floorRay.ray.diff = { 0.0f, -10.0f - rayHitPalamata_.rayOffset, 0.0f };
    rayList_.push_back(floorRay);

    float wallLength = Radius + 0.2f;

    CollisionRayInfo frontRay;
    frontRay.name = "FrontWall";
    frontRay.ray.diff = { 0.0f, 0.0f, wallLength };
    rayList_.push_back(frontRay);

    CollisionRayInfo backRay;
    backRay.name = "BackWall";
    backRay.ray.diff = { 0.0f, 0.0f, -wallLength };
    rayList_.push_back(backRay);

    CollisionRayInfo leftRay;
    leftRay.name = "LeftWall";
    leftRay.ray.diff = { -wallLength, 0.0f, 0.0f };
    rayList_.push_back(leftRay);

    CollisionRayInfo rightRay;
    rightRay.name = "RightWall";
    rightRay.ray.diff = { wallLength, 0.0f, 0.0f };
    rayList_.push_back(rightRay);
}

const CollisionRayInfo* Player::GetRayInfo(const std::string& name) const {
    for (const auto& rayInfo : rayList_) {
        if (rayInfo.name == name) {
            return &rayInfo;
        }
    }
    return nullptr;
}

void Player::RayCastUpdate()
{
    UpdateRayCollisions();
}

void Player::UpdateRayCollisions()
{
    if (!scene_) return;
    auto gs = dynamic_cast<GameScene*>(scene_);
    if (!gs) return;

    const std::vector<Triangle>& triangles = gs->GetTriangle();

    Vector3 center = object_->GetTranslate();

    // 【修正点1】壁レイの発射方向を「現在の進行方向（ワールド絶対）」と「左右方向」にする
    // プレイヤーの回転行列依存にすると衝突時にレイも回転して挙動が不安定になるため、
    // レール進行方向(railDir)を基点にします。
    Vector3 railDir = railMover_->GetCurrentDirection();
    Vector3 forwardDir = (Length(railDir) > 0.001f) ? Normalize(railDir) : Vector3{ 0.0f, 0.0f, 1.0f };

    // ワールドY軸(0,1,0)との外積でプレイヤーの右方向ベクトルを求める
    Vector3 rightDir = Normalize(Cross({ 0.0f, 1.0f, 0.0f }, forwardDir));

    float wallLength = Radius + 0.3f; // レイの長さ（半径 + マージン）

    // 衝突によるレールの押し戻し量
    float maxPushBackProgress = 0.0f;

    for (auto& rayInfo : rayList_) {
        rayInfo.isColide = false;
        rayInfo.distance = FLT_MAX;
        rayInfo.crossPoint = {};
        rayInfo.hitNormal = {};
        rayInfo.hitTriangle = {};

        // レイの起点と方向を設定
        if (rayInfo.name == "Floor") {
            rayInfo.ray.origin = center;
            rayInfo.ray.origin.y += rayHitPalamata_.rayOffset;
            rayInfo.ray.diff = { 0.0f, -10.0f - rayHitPalamata_.rayOffset, 0.0f };
        } else if (rayInfo.name == "FrontWall") {
            rayInfo.ray.origin = center;
            rayInfo.ray.origin.y += kHeightOffset;
            rayInfo.ray.diff = Multiply(wallLength, forwardDir);
        } else if (rayInfo.name == "BackWall") {
            rayInfo.ray.origin = center;
            rayInfo.ray.origin.y += kHeightOffset;
            rayInfo.ray.diff = Multiply(-wallLength, forwardDir);
        } else if (rayInfo.name == "LeftWall") {
            rayInfo.ray.origin = center;
            rayInfo.ray.origin.y += kHeightOffset;
            rayInfo.ray.diff = Multiply(-wallLength, rightDir);
        } else if (rayInfo.name == "RightWall") {
            rayInfo.ray.origin = center;
            rayInfo.ray.origin.y += kHeightOffset;
            rayInfo.ray.diff = Multiply(wallLength, rightDir);
        }

        if (triangles.empty()) continue;

        for (const auto& tri : triangles) {
            Vector3 tmpHit = {};
            float dist = 0.0f;
            RayTriangleCollisionResult result;

            if (CheckRayTriangle(rayInfo.ray, tri, &dist, &tmpHit, &result)) {
                // 【修正点2】FrontFace / BackFace 両方で当たり判定を取る
                if (result == RayTriangleCollisionResult::FrontFace || result == RayTriangleCollisionResult::BackFace) {

                    Vector3 v01 = Subtract(tri.vertices[1], tri.vertices[0]);
                    Vector3 v12 = Subtract(tri.vertices[2], tri.vertices[1]);
                    Vector3 normal = Normalize(Cross(v01, v12));

                    // 裏面衝突時は法線を裏返す
                    if (result == RayTriangleCollisionResult::BackFace) {
                        normal = Multiply(-1.0f, normal);
                    }

                    // 床・天井などの傾斜面を弾く（ほぼ垂直な壁のみ壁判定とする）
                    if (rayInfo.name != "Floor" && std::abs(normal.y) >= 0.7f) {
                        continue;
                    }

                    // 一番近い交差を採用
                    if (dist < rayInfo.distance) {
                        rayInfo.distance = dist;
                        rayInfo.crossPoint = tmpHit;
                        rayInfo.hitNormal = normal;
                        rayInfo.hitTriangle = tri;
                        rayInfo.isColide = true;
                    }
                }
            }
        }

        // デバッグ描画
        float drawScale = (rayInfo.name == "Floor") ? 1.0f : 1.0f;
        Vector3 drawEnd = Add(rayInfo.ray.origin, Multiply(drawScale, rayInfo.ray.diff));
        PrimitiveDrawer::GetInstance()->DrawLine(rayInfo.ray.origin, drawEnd,
            rayInfo.isColide ? Vector4{ 1,0,0,1 } : Vector4{ 0,1,0,1 });

        if (rayInfo.isColide) {
            PrimitiveDrawer::GetInstance()->DrawSphere({ rayInfo.crossPoint, 0.05f, {} }, { 0,0,1,1 });

            // 【修正点3】めり込み量の計算と押し戻し
            if (rayInfo.name == "FrontWall") {
                // レイの先端から交差地点までの距離＝めり込み量
                float penetration = wallLength - rayInfo.distance;
                if (penetration > 0.0f) {
                    // 前方の壁にぶつかったら進行方向と逆（後方）へ押し戻す
                    maxPushBackProgress = (std::max)(maxPushBackProgress, penetration);
                }
            } else if (rayInfo.name == "BackWall") {
                float penetration = wallLength - rayInfo.distance;
                if (penetration > 0.0f) {
                    // 後方の壁にぶつかったら進行方向（前方）へ押し戻す
                    maxPushBackProgress = (std::min)(maxPushBackProgress, -penetration);
                }
            }
        }
    }

    // 【修正点4】めり込んだ分だけレールの進捗（Progress）を戻す
    if (std::abs(maxPushBackProgress) > 0.0001f) {
        // 壁にぶつかった距離だけレールを押し戻して止める
        railMover_->Advance(-maxPushBackProgress);
    }

    // レール上ゲームのため、位置自体の直接的なオフセット加算は行わない（向きが狂う原因になるため）
    wallPushOffset_ = { 0.0f, 0.0f, 0.0f };

    // 地面判定の更新
    auto floorRay = GetRayInfo("Floor");
    if (floorRay) {
        isRayHit_ = floorRay->isColide;
        rayHitDistance_ = floorRay->distance;
        rayHitPoint_ = floorRay->crossPoint;
        rayHitTriangle_ = floorRay->hitTriangle;
        result_ = RayTriangleCollisionResult::FrontFace;
    }
}
void Player::HandleInput()
{
    // ノックバック中は操作不能にする
    if (isKnockback_) {
        return;
    }

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

void Player::HandleKnockback()
{
    if (isKnockback_ && knockbackTimer_ > 0.0f) {
        // ノックバックの残り時間に応じて速度を減衰
        float progressRatio = knockbackTimer_ / kKnockbackDuration_;
        float moveAmount = float(knockbackDirection_) * kKnockbackSpeed_ * progressRatio * deltaTime_;
        railMover_->Advance(moveAmount);

        // Advance内でcurrentDir_が反転してしまわないよう、保存した移動向きを維持
        railMover_->SetMoveDirection(savedFacingDirection_);

        knockbackTimer_ -= deltaTime_;
        if (knockbackTimer_ <= 0.0f) {
            knockbackTimer_ = 0.0f;
            isKnockback_ = false;
        }
    }
}

void Player::TakeDamage(int knockbackDirection)
{
    if (hitInvincibilityTimer_ <= 0.0f && !isInvincible_) {
        isDamaged_ = true;
        hitInvincibilityTimer_ = kHitInvincibilityDuration_;
        hitPoints_.value--;

        // ノックバック開始
        isKnockback_ = true;
        knockbackTimer_ = kKnockbackDuration_;
        knockbackDirection_ = knockbackDirection;
        velocity_.y = kKnockbackJumpForce_;
        isGrounded_ = false;

        // ノックバック前の移動向きを保存
        if (railMover_) {
            savedFacingDirection_ = railMover_->GetMoveDirection();
        }

        // アタック中などの特殊状態を解除して通常行動に戻す
        if (baseState_ && baseState_->GetFactory()) {
            baseState_->ChangeBehavior(this, baseState_->GetFactory()->CreateBehavior(BehaviorType::Root));
        }
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

    if (isKnockback_) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "STATUS: KNOCKBACK! (Timer: %.2f)", knockbackTimer_);
    } else if (isDamaged_) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "STATUS: INVINCIBLE / DAMAGED!");
    } else {
        ImGui::Text("STATUS: Normal");
    }

    ImGui::Text("Hit Points: %d", hitPoints_.value);
    ImGui::ProgressBar(hitInvincibilityTimer_ / kHitInvincibilityDuration_, ImVec2(0, 0), "Hit Timer");

    ImGui::Separator();
    ImGui::Text("--- Raycast Info ---");
    for (const auto& rayInfo : rayList_) {
        if (rayInfo.isColide) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[%s] Hit! Dist: %.2f", rayInfo.name.c_str(), rayInfo.distance);
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[%s] No Hit", rayInfo.name.c_str());
        }
    }

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

            // プレイヤーが攻撃中の場合
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

            // 敵が通常状態の場合、ダメージおよびノックバック
            if (enemyState && strcmp(enemyState, "Normal") == 0) {
                if (hitInvincibilityTimer_ <= 0.0f) {
                    // レール上の位置関係からノックバック方向を決定
                    int knockDir = -1;
                    float playerDist = GetCurrentDistance();
                    float enemyDist = enemy->GetCurrentDistance();
                    if (playerDist < enemyDist) {
                        knockDir = -1; // プレイヤーが手前 → 手前側(後退)へノックバック
                    } else if (playerDist > enemyDist) {
                        knockDir = 1;  // プレイヤーが奥 → 奥側(前進)へノックバック
                    } else {
                        // 同一地点の場合はプレイヤーの移動向きの逆方向
                        knockDir = (GetMoveDirection() == 1) ? -1 : 1;
                    }

                    TakeDamage(knockDir);
                }
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