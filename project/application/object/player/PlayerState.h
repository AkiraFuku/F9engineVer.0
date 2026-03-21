#pragma once

#include<Vector4.h>

class Player;

// ======================================
// 基底クラス
// ======================================
class IPlayerState
{
public:
    virtual ~IPlayerState() = default;
    // 初期化
    virtual void Initialize(Player* player) = 0;
    // 更新
    virtual void Update(Player* player) = 0;
};

// ======================================
// 待機状態
// ======================================
class PlayerStateIdle : public IPlayerState {
public:
    // 初期化
    void Initialize(Player* player)override;
    // 更新
    void Update(Player* player)override;
};

// ======================================
// 移動状態
// ======================================
class PlayerStateMove : public IPlayerState {
public:
    // 初期化
    void Initialize(Player* player) override;
    // 更新
    void Update(Player* player) override;
};

// ======================================
// 発射状態
// ======================================
class PlayerStateShoot : public IPlayerState {
public:
    // 初期化
    void Initialize(Player* player) override;
    // 更新
    void Update(Player* player) override;

private:
    // 発射直後の硬直時間を管理(単位: 秒)
    float frameCount_ = 0;

    static inline const float kDeltaTime = 1.0f / 60.0f;
};