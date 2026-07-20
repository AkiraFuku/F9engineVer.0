#pragma once
#include <memory>
#include "BehaviorState.h"
class Enemy;
class IEnemyAction;

class IEnemyBehavior :public IBehavior {
public:
    virtual ~IEnemyBehavior() = default; // 基底クラスの仮想デストラクタ
    virtual void Initialize(Enemy* enemy) = 0;
    virtual void Update(Enemy* enemy) = 0;
    virtual void Finalize(Enemy* enemy) = 0;

};
class EnemyBehaviorPatrol : public IEnemyBehavior {
public:

    EnemyBehaviorPatrol()=default;  // コンストラクタを宣言
    ~EnemyBehaviorPatrol(); // ★デストラクタをここで宣言（インライン実装しない）

    void Initialize(Enemy* enemy) override;
    void Update(Enemy* enemy) override;
    void Finalize(Enemy* enemy) override;
    const char* GetName() const override {
        return "Patrol";
    }
private:

    std::unique_ptr<IEnemyAction> currentAction_;

};