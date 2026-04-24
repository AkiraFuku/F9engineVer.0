#pragma once
#pragma once
#include <memory>

class Player;
class Enemy;
class ICommand;

// アクション（行動）の基底クラス
class IBehavior {
public:
    virtual ~IBehavior() = default;
    virtual const char* GetName() const = 0; // 追加
};

