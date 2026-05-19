#pragma once
#include <memory>
#include <string>

class IStateRideOn; // 前方宣言

class Robot {
public:
    Robot();
    virtual ~Robot();
    virtual std::unique_ptr<IStateRideOn> CreateRideOnState() = 0;
  

    virtual std::string GetName() const = 0;
};