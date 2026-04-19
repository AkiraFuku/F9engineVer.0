#pragma once
#include <memory>
#include <vector>
class Player;
class ICommand;
// --- 行動ビヘイビア (Action) ---
// 物理計算や移動ロジックを担当
class IPlayerAction;

// --- 状態ビヘイビア (State) ---
// どの行動を組み合わせて使うか、どの状態へ遷移するかを担当
class IPlayerState {
public:
    virtual ~IPlayerState() = default;
    virtual void Initialize(Player* player) = 0;
    virtual void Update(Player* player) = 0;
    virtual void Finalize(Player* player) = 0;
    virtual void HandleInput(Player* player, ICommand* command) = 0;
    virtual const char* GetName() const = 0; // 追加
};
//通常状態
class StateNormal : public IPlayerState {
public:
    StateNormal();
    ~StateNormal();
    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void Finalize(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;
    const char* GetName() const override { return "Normal"; } // StateNormalの場合

private:
    std::unique_ptr<IPlayerAction> moveAction_;
    std::unique_ptr<IPlayerAction> attackAction_;
    std::unique_ptr<IPlayerAction> jumpAction_;
};

//搭乗状態
class StateRideOn : public IPlayerState {
public:
   
    ~StateRideOn();

   
    // ロボットの種類ごとにアクションを注入する
    StateRideOn(std::unique_ptr<IPlayerAction> moveAction,
        std::unique_ptr<IPlayerAction> attackAction);
       
    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void Finalize(Player* player) override;
    void HandleInput(Player* player, ICommand* command) override;
    const char* GetName() const override { return "RideOn"; } // StateNormalの場合
private:
    std::unique_ptr<IPlayerAction> moveAction_;
    std::unique_ptr<IPlayerAction> attackAction_;
    std::unique_ptr<IPlayerAction> jumpAction_;

};