#pragma once
#include "Robot.h"
#include <memory>
#include <string>
#include "PlayerState.h"
#include "PlayerBehavior.h"
class Player;

class IStateRideOn;

/// <summary>
/// バウンドエネミー用ロボットクラス
/// プレイヤーが乗っ取った際に StateRideOnBounce を提供
/// </summary>
class BoundRobot :
    public Robot
{

public:
    BoundRobot();
    ~BoundRobot() override;

    std::unique_ptr<IStateRideOn> CreateRideOnState() override;
    std::string GetName() const override { return "BoundRobot"; }
};
// バウンド能力専用の Root ビヘイビア（接地即ジャンプを担当）
class BehaviorBounceRoot : public BehaviorRoot {
public:
    void Initialize(Player* player) override;
    void Update(Player* player) override;
    const char* GetName() const override { return "BounceRoot"; }
};

// バウンド能力（ホバリングジャンプ）を持った搭乗ステート
class StateRideOnBounce : public IStateRideOn {
public:
    StateRideOnBounce();
    ~StateRideOnBounce() = default;

    void Initialize(Player* player) override;
    void BehaviorUpdate(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;
    
    const char* GetName() const override {
        return "RideOnBounce";
    }

    IPlayerAction* GetMoveAction() override { return moveAction_.get(); }
    IPlayerAction* GetJumpAction() override { return jumpAction_.get(); }
    IPlayerAction* GetAttackAction_() override { return attackAction_.get(); }
};