#include "EnemyAction.h"
#include "Enemy.h"
void MoveAction::Execute(Enemy* enemy){
    // Player.cpp にあった移動ロジックをここに移譲
    enemy->Move(speed_);
}

void JumpAction::Execute(Enemy* enemy){
    if (enemy&&enemy->IsGround())
    {
        enemy->SetVelocity({0.0f,acceleration_,0.0f});

    }

}
//攻撃
void AttackAction::Execute(Enemy* enemy)
{
    enemy;
}
//特殊技
void SkillAction::Execute(Enemy* enemy)
{
    enemy;
}
