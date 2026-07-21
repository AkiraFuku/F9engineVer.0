#pragma once
class Enemy;
enum EnemyActionType
{
    Jump,
    move,
    Attack


};
// --- 行動ビヘイビア (Action) ---
// 物理計算や移動ロジックを担当
class IEnemyAction {
public:
    virtual ~IEnemyAction() = default;
    virtual void Execute(Enemy* enemy) = 0;
private:
   // EnemyActionType EnemyType;
};
class MoveAction : public IEnemyAction {
public:
    MoveAction(float speed) : speed_(speed) {}
    void Execute(Enemy* enemy) override ;
private:
    float speed_;
};

// ジャンプアクション
class JumpAction : public IEnemyAction {
public:
    void SetAcceleration(float acceleration){
        acceleration_=acceleration;
    };

    void Execute(Enemy* enemy) override;
private:
    float acceleration_ = 24.0f; 
};
class AttackAction : public IEnemyAction {
public:
    void Execute(Enemy* enemy) override;
};
class SkillAction : public IEnemyAction {
public:
    void Execute(Enemy* enemy) override;
};
