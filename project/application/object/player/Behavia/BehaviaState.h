#pragma once
#pragma once
#include <memory>

class Player;
class ICommand;

// アクション（行動）の基底クラス
class IBehaviorState {
public:
    virtual ~IBehaviorState() = default;
    virtual void Initialize(Player* player) = 0;
    virtual void Update(Player* player) = 0;
    // その状態で特定のコマンドを受け付けるかどうかを判定し、実行する
    virtual void HandleInput(Player* player, ICommand* command) = 0;
    virtual const char* GetName() const = 0; // 追加
};

// --- 具体的な行動状態 ---

// 通常時（移動・ジャンプ・攻撃への遷移が可能）
class BehaviorRoot : public IBehaviorState {
public:
    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;
    const char* GetName() const override { return "Root"; } // BehaviorRootの場合
};

// 攻撃中（移動やジャンプを制限する）
class BehaviorAttack : public IBehaviorState {
public:
    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;
    const char* GetName() const override { return "Attack"; } // BehaviorRootの場合
private:
    int timer_ = 0;
    const int kAttackDuration = 30; // 攻撃の持続時間
};

// ジャンプ中（左右移動はできるが、再度ジャンプはできない等）
class BehaviorJump : public IBehaviorState {
public:
    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;
    const char* GetName() const override { return "Jump"; } // BehaviorRootの場合
};