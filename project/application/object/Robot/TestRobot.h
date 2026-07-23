#pragma once
#include "Robot.h"
#include <memory>
#include <string>

class IPlayerFactory;

/// <summary>
/// テスト用ロボットクラス
/// StateRideOnTestを保持し、プレイヤーが乗っ取る際に提供
/// </summary>
class TestRobot : public Robot {
public:
    TestRobot();
    ~TestRobot();
   std::shared_ptr<IPlayerFactory>CreatePlayerFactory()override;

    std::string GetName() const override { return "TestRobot"; }

private:
};
