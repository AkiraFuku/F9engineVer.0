#pragma once
class Player;
enum ActionType
{
    Jump,
    move,
    Attack


};
// --- 行動ビヘイビア (Action) ---
// 物理計算や移動ロジックを担当
class IPlayerAction {
public:
    virtual ~IPlayerAction() = default;
    virtual void Execute(Player* player) = 0;
private:
   // ActionType Type;
};
class NormalMoveAction : public IPlayerAction {
public:
    NormalMoveAction(float speed) : speed_(speed) {}
    void Execute(Player* player) override ;
private:
    float speed_;
};

// ジャンプアクション
class NormalJumpAction : public IPlayerAction {
public:
    void Execute(Player* player) override;
};
