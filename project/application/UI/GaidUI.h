#pragma once
#include "Vector2.h"
#include <memory>
#include <string>

class Player;
class Sprite;

class GaidUI
{
public:
    GaidUI() = default;
    ~GaidUI() = default;

    void Initialize(Player* player);
    void Update();
    void Draw();

    // プレイヤー設定
    void SetPlayer(Player* player) { player_ = player; }

    // ガイドUI全体の基準位置・サイズ設定
    void SetBasePosition(const Vector2& position) { basePosition_ = position; }
    void SetGuideSize(const Vector2& size) { guideSize_ = size; }
    void SetSpacing(float spacing) { spacing_ = spacing; }

    const Vector2& GetBasePosition() const { return basePosition_; }

private:
    Player* player_ = nullptr;

    // 各操作ガイド用スプライト
    std::unique_ptr<Sprite> moveSprite_ = nullptr;
    std::unique_ptr<Sprite> attackSprite_ = nullptr;
    std::unique_ptr<Sprite> jumpSprite_ = nullptr;
    std::unique_ptr<Sprite> shootSprite_ = nullptr;
    std::unique_ptr<Sprite> aimSprite_ = nullptr;

    // 表示フラグ
    bool isAimMode_ = false;

    // 配置・サイズ設定（画面右下付近を想定）
    Vector2 basePosition_ = { 0.0f, 620.0f }; // 基準座標
    Vector2 guideSize_ = { 256.0f, 96.0f };       // 各スプライトのサイズ
    float spacing_ = 320.0f;                      // アイコン同士の間隔
};
