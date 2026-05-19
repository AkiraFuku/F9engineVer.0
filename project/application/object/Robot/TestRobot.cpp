#include "TestRobot.h"
#include "PlayerState.h"

TestRobot::TestRobot() {
    
}

TestRobot::~TestRobot() = default;

std::unique_ptr<IStateRideOn> TestRobot::CreateRideOnState()
{
    return std::make_unique<StateRideOnTest>();
}