#pragma once
#include "Vector2.h"
#include <memory>
#include <vector>
class Player;
class Sprite;

/// <summary>
/// プレイヤーのHPを表示するUIクラス
/// </summary>
class PlayerHPUI
{
public:
    void Initialize( Player* player);
    void Update();
    void Draw();

    void SetPlayer(Player* player) {
        player_ = player;
    }

private:

    Vector2 position_ = { 0.0f, 0.0f }; // HP UIの位置



    //プレイヤー
    Player* player_ = nullptr;
    //HPの背景スプライト用
    std::unique_ptr<Sprite> hpSprite_ = nullptr;
    //HPのカウンタースプライト用
    std::vector<std::unique_ptr<Sprite>> CounterSprite_ ;
};

