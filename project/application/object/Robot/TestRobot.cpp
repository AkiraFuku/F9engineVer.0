#include "TestRobot.h"
#include "PlayerState.h"

TestRobot::TestRobot() {
    // StateRideOnTestを生成してPlayerState_に設定
    PlayerState_ = std::make_unique<StateRideOnTest>();
}

TestRobot::~TestRobot() = default;

