// Command.h
#pragma once

class Player;

// コマンドのインターフェース
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void Execute(Player& player) = 0;
};

// 移動コマンド
class MoveCommand : public ICommand {
public:
    MoveCommand(float speed) : speed_(speed) {}
    void Execute(Player& player) override; // 後述するPlayerのメソッドを呼ぶ
private:
    float speed_;
};

// ジャンプコマンド
class JumpCommand : public ICommand {
public:
    void Execute(Player& player) override;
};

