#include "EnemyBehavior.h"
#include "Enemy.h"
#include "EnemyAction.h"
void EnemyBehaviorPatrol::Initialize(Enemy* enemy)
{
    currentAction_= std::make_unique<MoveAction>(1.0f);
}
EnemyBehaviorPatrol::~EnemyBehaviorPatrol() = default;
void EnemyBehaviorPatrol::Update(Enemy* enemy)
{// 1. Actionを生成して実行する
    // 例: 毎フレーム速度 1.0f で移動するアクションを実行
    currentAction_->Execute(enemy);
}

void EnemyBehaviorPatrol::Finalize(Enemy* enemy)
{
}
