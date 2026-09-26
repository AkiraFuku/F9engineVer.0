#include "ChaseEnemy.h"
#include "TestRobot.h"
#include "Robot.h"
#include "EnemyBehavior.h"
#include "EnemyState.h"
#include "PrimitiveDrawer.h"
#include "imgui.h"

ChaseEnemy::ChaseEnemy() {}
ChaseEnemy::~ChaseEnemy() = default;

void ChaseEnemy::Initialize() {
    Enemy::Initialize();

    // ユーザー指定の taru3.png を適用
    object_->SetTexture("resources/taru/taru3.png");

    // 初期移動速度を巡回速度に設定
    SetMoveSpeed(patrolSpeed_);

    // 追跡専用ビヘイビアを設定
    ChangeBehavior(std::make_unique<EnemyBehaviorChase>());

    // 撃破時に出現するロボットを設定
    SetRobot(std::make_unique<TestRobot>());
}

void ChaseEnemy::Update() {
    Enemy::Update();
}

void ChaseEnemy::Draw() {
    Enemy::Draw();

#ifdef USE_LINE
    // デバッグ用：検知範囲スフィアの可視化（追跡中は赤、索敵中は黄色）
    Sphere searchSphere;
    searchSphere.center = GetWorldPosition();
    searchSphere.radius = isChasing_ ? lostDistance_ : searchRadius_;
    Vector4 sphereColor = isChasing_ ? Vector4{ 1.0f, 0.2f, 0.2f, 0.6f } : Vector4{ 1.0f, 0.9f, 0.2f, 0.4f };
    PrimitiveDrawer::GetInstance()->DrawSphere(searchSphere, sphereColor);
#endif // USE_LINE
}
