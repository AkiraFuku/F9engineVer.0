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
class IEnemyBehavior:public IBehavior{
      virtual void Initialize(Enemy* enemy) = 0;
    virtual void Update(Enemy* enemy) = 0;
    virtual void Finalize(Enemy* enemy)=0;
    // その状態で特定のコマンドを受け付けるかどうかを判定し、実行する
    };

