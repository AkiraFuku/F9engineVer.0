#include "TestRobot.h"
#include "IPlayerFactory.h" // RideOnPlayerFactory の定義
TestRobot::TestRobot() {
    
}

TestRobot::~TestRobot() = default;

std::shared_ptr<IPlayerFactory> TestRobot::CreatePlayerFactory()
{
    // このロボットに対応する RideOnPlayerFactory を生成して渡す
    return std::make_shared<RideOnPlayerFactory>();
}