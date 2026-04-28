#pragma once
#include "BehaviorState.h"
class Player;
class ICommand;
// アクション（行動）の基底クラス

class IPlayerBehavior : public IBehavior {
public:
    virtual void Initialize(Player* player) = 0;
    virtual void Update(Player* player) = 0;
    virtual void Finalize(Player* player) = 0;
    // その状態で特定のコマンドを受け付けるかどうかを判定し、実行する
    virtual void HandleInput(Player* player, ICommand* command) = 0;
};




// --- 具体的な行動状態 ---

// 通常時（移動・ジャンプ・攻撃への遷移が可能）
class BehaviorRoot : public IPlayerBehavior {
public:
    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void Finalize(Player* player)override;
    void HandleInput(Player* player, ICommand* command) override;
    const char* GetName() const override {
        return "Root";
    } // BehaviorRootの場合
};

// 攻撃中（移動やジャンプを制限する）
class BehaviorAttack : public IPlayerBehavior {
public:
    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void Finalize(Player* player)override;
    void HandleInput(Player* player, ICommand* command) override;
    const char* GetName() const override {
        return "Attack";
    } // BehaviorRootの場合
private:
    int timer_ = 0;
    const int kAttackDuration = 30; // 攻撃の持続時間
};

// ジャンプ中（左右移動はできるが、再度ジャンプはできない等）
class BehaviorJump : public IPlayerBehavior {
public:
    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void Finalize(Player* player)override;
    void HandleInput(Player* player, ICommand* command) override;
    const char* GetName() const override {
        return "Jump";
    } // BehaviorRootの場合
};
class BehaviorAim : public IPlayerBehavior {
public:
    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void Finalize(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;
    const char* GetName() const override { return "Aim"; }

private:
    float aimX_ = 1.0f; // デフォルトは右向き
    float aimY_ = 0.0f;
    const float kAimFallSpeed = 0.02f; // ゆっくり降下する速度
};