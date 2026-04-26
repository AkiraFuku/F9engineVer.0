#pragma once
class Player;
enum ActionType
{
    kJump,
    kMove,
    kAttack


};
// --- 行動ビヘイビア (Action) ---
// 物理計算や移動ロジックを担当
class IPlayerAction {
public:
    virtual ~IPlayerAction() = default;
    virtual void Execute(Player* player) = 0;
private:
  
};
class NormalMoveAction : public IPlayerAction {
public:
    NormalMoveAction(float speed) : speed_(speed) {}
    void Execute(Player* player) override ;
    void SetSpeed(float speed){speed_=speed;}
private:
    float speed_;
      ActionType Type=kMove;
};

// ジャンプアクション
class NormalJumpAction : public IPlayerAction {
public:
    void Execute(Player* player) override;
public:
    ActionType Type=kJump;
};
