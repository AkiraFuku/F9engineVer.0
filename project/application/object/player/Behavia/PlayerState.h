#pragma once

class Player;
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
};
//通常状態
class StateNormal : public IPlayerState {
public:
    void Initialize(Player* player) override;
    void Update(Player* player) override;
    void Finalize(Player* player) override;

    void Move();
    void Attack();
};

//搭乗状態
class StateRideOn : public IPlayerState {
public:
     void Initialize(Player* player) override;
    void Update(Player* player) override;
    void Finalize(Player* player) override;

    void Move();
    void Attack();

};