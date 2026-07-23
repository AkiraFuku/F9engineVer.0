#pragma once
#include <memory>
#include <string>

class IPlayerFactory; // 前方宣言

class Robot {
public:
    Robot() = default;
    virtual ~Robot() = default;

    // ロボットが持つ Factory を取得する
    virtual std::shared_ptr<IPlayerFactory> CreatePlayerFactory() = 0;

    virtual std::string GetName() const = 0;
};