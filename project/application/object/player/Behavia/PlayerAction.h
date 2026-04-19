#pragma once
class Player;
// --- 行動ビヘイビア (Action) ---
// 物理計算や移動ロジックを担当
class IPlayerAction {
public:
    virtual ~IPlayerAction() = default;
    virtual void Execute(Player* player) = 0;
};
