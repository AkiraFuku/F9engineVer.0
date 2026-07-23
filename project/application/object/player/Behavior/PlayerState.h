#pragma once
#include <memory>
#include <vector>
#include "PlayerBehavior.h"
#include "PlayerAction.h"

class Player;
class ICommand;

// --- 状態ビヘイビア (State) ---
class IPlayerState {
public:
    IPlayerState() = default;
    virtual ~IPlayerState() = default;

    virtual void Initialize(Player* player) = 0;
    virtual void Update(Player* player);
    virtual void BehaviorUpdate(Player* player) = 0;
    virtual void Finalize(Player* player);
    virtual void HandleInput(Player* player, ICommand* command);
    virtual const char* GetName() const = 0;

    virtual IPlayerAction* GetMoveAction() = 0;
    virtual IPlayerAction* GetJumpAction() = 0;
    virtual IPlayerAction* GetAttackAction_() = 0;

    // --- State 内で安全に Behavior を切り替える設計 ---
    void ChangeBehavior(Player* player, std::unique_ptr<IPlayerBehavior> next);

    IPlayerBehavior* GetBehavior() {
        return currentBehavior_.get();
    }

protected:
    std::unique_ptr<IPlayerBehavior> currentBehavior_ = nullptr;
};

// 通常状態
class StateNormal : public IPlayerState {
public:
    StateNormal() = default;
    ~StateNormal() override = default;

    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void BehaviorUpdate(Player* player) override;
    void Finalize(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;

    const char* GetName() const override { return "Normal"; }

    IPlayerAction* GetMoveAction() override { return moveAction_.get(); }
    IPlayerAction* GetJumpAction() override { return jumpAction_.get(); }
    IPlayerAction* GetAttackAction_() override { return attackAction_.get(); }

private:
    std::unique_ptr<IPlayerAction> moveAction_ = nullptr;
    std::unique_ptr<IPlayerAction> attackAction_ = nullptr;
    std::unique_ptr<IPlayerAction> jumpAction_ = nullptr;
};

// 搭乗状態
class IStateRideOn : public IPlayerState {
public:
    IStateRideOn(std::unique_ptr<IPlayerAction> move, std::unique_ptr<IPlayerAction> jump,
                 std::unique_ptr<IPlayerAction> attack, std::unique_ptr<IPlayerAction> shoot);
    ~IStateRideOn() override = default;

    void Initialize(Player* player) override = 0;
    void Finalize(Player* player) override;
    void Update(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;

    void DoMove(Player* player);
    void DoShoot(Player* player);

    IPlayerAction* GetShootAction() { return shootAction_.get(); }

protected:
    std::unique_ptr<IPlayerAction> moveAction_ = nullptr;
    std::unique_ptr<IPlayerAction> attackAction_ = nullptr;
    std::unique_ptr<IPlayerAction> jumpAction_ = nullptr;
    std::unique_ptr<IPlayerAction> shootAction_ = nullptr;
};

// 仮のライドオンステート派生クラス
class StateRideOnTest : public IStateRideOn {
public:
    StateRideOnTest();
    ~StateRideOnTest() override = default;

    void Initialize(Player* player) override;
    void BehaviorUpdate(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;

    const char* GetName() const override { return "RideOnTest"; }

    IPlayerAction* GetMoveAction() override { return moveAction_.get(); }
    IPlayerAction* GetJumpAction() override { return jumpAction_.get(); }
    IPlayerAction* GetAttackAction_() override { return attackAction_.get(); }
};

// 死亡状態
class StateDead : public IPlayerState {
public:
    StateDead() = default;
    ~StateDead() override = default;

    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void BehaviorUpdate(Player* player) override;
    void Finalize(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;

    const char* GetName() const override { return "Dead"; }

    IPlayerAction* GetMoveAction() override { return nullptr; }
    IPlayerAction* GetJumpAction() override { return nullptr; }
    IPlayerAction* GetAttackAction_() override { return nullptr; }
};