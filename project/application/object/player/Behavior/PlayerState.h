#pragma once
#include <memory>
#include <vector>
#include "PlayerBehavior.h"
#include "PlayerAction.h"
class Player;
class ICommand;

// --- 状態ビヘイビア (State) ---
// どの行動を組み合わせて使うか、どの状態へ遷移するかを担当
class IPlayerState {
public:
     IPlayerState() = default;
    virtual ~IPlayerState() = default;
    virtual void Initialize(Player* player) = 0;
   // State共通のUpdate。現在のBehaviorを更新する
    virtual void Update(Player* player);
    virtual void  BehaviorUpdate(Player* player) = 0;
    virtual void Finalize(Player* player) = 0;
    virtual void HandleInput(Player* player, ICommand* command) ;
    virtual const char* GetName() const = 0; // 追加
    virtual IPlayerAction* GetMoveAction() = 0;
    virtual IPlayerAction* GetJumpAction() = 0;
    virtual IPlayerAction* GetAttackAction_() = 0;

    void ChangeBehavior(std::unique_ptr<IPlayerBehavior> next) {
        currentBehavior_ = std::move(next);
    }
    IPlayerBehavior* GetBehavior() {
        return currentBehavior_.get();
    }

protected:
    std::unique_ptr<IPlayerBehavior> currentBehavior_; // StateがBehaviorを管理
};
//通常状態
class StateNormal : public IPlayerState {
public:
    StateNormal()=default;
    ~StateNormal() = default;
    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void  BehaviorUpdate(Player* player)override;
    void Finalize(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;
    const char* GetName() const override {
        return "Normal";
    } // StateNormalの場合

    IPlayerAction* GetMoveAction()override {
        return moveAction_.get();
    }
    IPlayerAction* GetJumpAction()override {
        return jumpAction_.get();
    }
    IPlayerAction* GetAttackAction_()override {
        return attackAction_.get();
    }

private:
    std::unique_ptr<IPlayerAction> moveAction_ = nullptr;
    std::unique_ptr<IPlayerAction> attackAction_ = nullptr;
    std::unique_ptr<IPlayerAction> jumpAction_ = nullptr;
};

//搭乗状態
class IStateRideOn : public IPlayerState {
public:
    // コンストラクタでアクションを注入できるようにする
    IStateRideOn(std::unique_ptr<IPlayerAction> move,std::unique_ptr<IPlayerAction> jump,std::unique_ptr<IPlayerAction> attack,
        std::unique_ptr<IPlayerAction> shoot);


    virtual ~IStateRideOn() = default;

    virtual void Initialize(Player* player)override = 0;
     void Finalize(Player* player)override ;

    virtual void Update(Player* player) override;

    // 共通の入力処理：基本的には現在の Behavior に任せる
    void HandleInput(Player* player, ICommand* command) override;

    // ここで共通のアクション実行メソッドを持っておくと便利
    void DoMove(Player* player);
    //ロボットを射出して通常状態に戻る
    void DoShoot(Player* player);

    // テスト用：射出アクション取得
    IPlayerAction* GetShootAction() {
        return shootAction_.get();
    }

protected:
    std::unique_ptr<IPlayerAction> moveAction_ = nullptr;
    std::unique_ptr<IPlayerAction> attackAction_ = nullptr;
    std::unique_ptr<IPlayerAction> jumpAction_ = nullptr;
    std::unique_ptr<IPlayerAction> shootAction_ = nullptr;

};

// 仮のライドオンステート派生クラス（テスト用）
class StateRideOnTest : public IStateRideOn {
public:
    StateRideOnTest();
    ~StateRideOnTest() = default;

    void Initialize(Player* player) override;
    void BehaviorUpdate(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;
    const char* GetName() const override {
        return "RideOnTest";
    }

    IPlayerAction* GetMoveAction() override {
        return moveAction_.get();
    }
    IPlayerAction* GetJumpAction() override {
        return jumpAction_.get();
    }
    IPlayerAction* GetAttackAction_() override {
        return attackAction_.get();
    }
};