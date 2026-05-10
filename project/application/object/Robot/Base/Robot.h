#pragma once
#include <memory>
#include <string>

class IStateRideOn; // 前方宣言

class Robot {
public:
    Robot();
    virtual ~Robot();
    virtual std::unique_ptr<IStateRideOn> CreateRideOnState() = 0;
    // プレイヤーに渡すための「ライドオンステート」を生成する
    // このステートの中に、そのロボット固有のアクションが詰め込まれている
  // std::unique_ptr<IStateRideOn>  PlayerState_=nullptr;

    virtual std::string GetName() const = 0;
};