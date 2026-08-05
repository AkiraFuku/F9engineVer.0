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

    std::shared_ptr<IPlayerFactory>CreatePlayerFactory()override;
    std::string GetName() const override {
        return "BoundRobot";
    }
};