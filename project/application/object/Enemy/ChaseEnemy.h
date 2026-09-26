#pragma once
#include "Enemy.h"

class ChaseEnemy : public Enemy {
public:
    ChaseEnemy();
    ~ChaseEnemy() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    // 追跡パラメータのゲッター・セッター
    float GetSearchRadius() const { return searchRadius_; }
    void SetSearchRadius(float radius) { searchRadius_ = radius; }

    float GetLostDistance() const { return lostDistance_; }
    void SetLostDistance(float dist) { lostDistance_ = dist; }

    float GetChaseSpeed() const { return chaseSpeed_; }
    void SetChaseSpeed(float speed) { chaseSpeed_ = speed; }

    float GetPatrolSpeed() const { return patrolSpeed_; }
    void SetPatrolSpeed(float speed) { patrolSpeed_ = speed; }

    bool IsChasing() const { return isChasing_; }
    void SetChasing(bool chasing) { isChasing_ = chasing; }

private:
    float searchRadius_ = 10.0f;  // プレイヤー検知・追跡開始範囲 (m)
    float lostDistance_ = 14.0f;  // 追跡解除・見失い距離 (m)
    float chaseSpeed_ = 5.5f;     // 追跡時の移動速度 (m/s)
    float patrolSpeed_ = 2.0f;    // 索敵・巡回時の移動速度 (m/s)
    bool isChasing_ = false;      // 現在追跡中かどうか
};
