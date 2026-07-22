#include "BoundRobot.h"
#include "Player.h"

BoundRobot::BoundRobot() {}
BoundRobot::~BoundRobot() = default;

std::unique_ptr<IStateRideOn> BoundRobot::CreateRideOnState() {
    return std::make_unique<StateRideOnBounce>();
}

