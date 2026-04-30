#pragma once
#include "../Base/Robot.h"
#include <memory>
#include <string>

class IStateRideOn;
class StateRideOnTest;

/// <summary>
/// テスト用ロボットクラス
/// StateRideOnTestを保持し、プレイヤーが乗っ取る際に提供
/// </summary>
class TestRobot : public Robot {
public:
    TestRobot();
    ~TestRobot();
    std::unique_ptr<IStateRideOn> CreateRideOnState()override;

    std::string GetName() const override { return "TestRobot"; }

private:
};
