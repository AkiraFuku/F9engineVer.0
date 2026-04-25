#pragma once
#pragma once
#include <memory>
#include <vector>
class Enemy;
class ICommand;
// --- 行動ビヘイビア (Action) ---
// 物理計算や移動ロジックを担当
class IEnemyAction;

// --- 状態ビヘイビア (State) ---
// どの行動を組み合わせて使うか、どの状態へ遷移するかを担当
class IEnemyState {
public:
    virtual ~IEnemyState() = default;
    virtual void Initialize(Enemy* enemy) = 0;
    virtual void Update(Enemy* enemy) = 0;
    virtual void Finalize(Enemy* enemy) = 0;
    virtual const char* GetName() const = 0; // 追加
};
//通常状態
class StateNormal : public IEnemyState {
public:
    StateNormal();
    ~StateNormal();
    void Initialize(Enemy* enemy) override;
    void Update(Enemy* enemy) override;
    void Finalize(Enemy* enemy) override;
    const char* GetName() const override {
        return "Normal";
    } // StateNormalの場合

private:
};

//搭乗状態
class IStateStan : public IEnemyState {
public:
 
    IStateStan();
    ~IStateStan() = default;


};