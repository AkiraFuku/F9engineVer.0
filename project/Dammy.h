#pragma once
#include "Enemy.h"
#include "Robot.h"
#include "PlayerState.h"

#include <vector>
#include "PlayerAction.h"

class DummyEnemy:public Enemy
{
public:

    //Enemyの種類によってロボットクラスの派生を渡す関数
    //Robot 持っているロボット（）｛return Robot｝
private:

};

class DummyRobot:public Robot{

public:
    //フィールドにおけるオブジェクトにしつつ変身状態（の定義を入れる）
    //プレイヤステートにDummyStateを入れる


private:
     std::unique_ptr<IStateRideOn>　playerState_=nullptr;


};
class DummyState:public IStateRideOn
{
public:
    /*
    moveAction_にDummyMoveActionを入れる
    
    */
private:

    std::unique_ptr<IPlayerAction> moveAction_=nullptr;
    std::unique_ptr<IPlayerAction> attackAction_=nullptr;
    std::unique_ptr<IPlayerAction> jumpAction_=nullptr;
    std::unique_ptr<IPlayerAction> shootAction_=nullptr;


};

class DummyMoveAction : public IPlayerAction {
public:
    DummyMoveAction(float speed) : speed_(speed) {}
    void Execute(Player* player) override ;
private:
    float speed_;
      ActionType Type=kMove;
};

// ジャンプアクション
class DummyJumpAction : public IPlayerAction {
public:
    void Execute(Player* player) override;
public:
    ActionType Type=kJump;
};


