#pragma once
#include "Vector2.h"
#include <memory>
#include <vector>
#include <string>

class Player;
class Sprite;

class ScoreUI
{
public:
    void Initialize(Player* player);
    void Update();
    void Draw();

    void SetPlayer(Player* player) { player_ = player; }

    // セッター
    void SetPosition(const Vector2& position) { position_ = position; }
    void SetCounterOffset(const Vector2& offset) { counterOffset_ = offset; }
    void SetCounterSpacing(float spacing) { counterSpacing_ = spacing; }
    void SetCounterSize(float size) { counterSize_ = size; }
    void SetScoreSpriteWidth(float width) { ScoreSpriteWidth_ = width; }

    // ゲッター
    const Vector2& GetPosition() const { return position_; }
    const Vector2& GetCounterOffset() const { return counterOffset_; }
    float GetCounterSpacing() const { return counterSpacing_; }
    float GetCounterSize() const { return counterSize_; }
    float GetScoreSpriteWidth() const { return ScoreSpriteWidth_; }

    void AddScore(int32_t score=1) {
        currentScore += score;
    }

private:
    // ---- 配置・デザイン用定数 ----
    static constexpr float kDefaultScale = 2.0f;              // UIの拡大率
    static constexpr float kDefaultPositionTop = 10.0f;        // 画面上部からの配置位置
    static constexpr float kMarginRight = 15.0f;               // 画面右端からのマージン
    static constexpr float kDefaultScoreSpriteWidth = 60.0f;  // スコアUIの背景幅
    static constexpr float kDefaultCounterSize = 20.0f;        // 数字のサイズ
    static constexpr float kDefaultCounterSpacing = 25.0f;     // 数字ごとの間隔

    // ---- スコアロジック用定数 ----
    static constexpr int32_t kMaxDigits = 2;                  // 表示する桁数[cite: 6]
    static constexpr int32_t kScoreResetBoundary = 100;       // リセット対象となる数値（100でリセット）
    static constexpr int32_t kRadixValue = 10;                 // 10進数の基数

    // 各桁のインデックス用列挙型
    enum DigitIndex {
        kTensPlace = 0, // 10の位
        kOnesPlace = 1, // 1の位
    };

    // ---- メンバ変数 ----
    Vector2 position_ = { 0.0f, 0.0f };
    float scale_ = kDefaultScale;

    // プレイヤー
    Player* player_ = nullptr;
    
    // 背景スプライト用
    std::unique_ptr<Sprite> ScoreSprite_ = nullptr;
    std::unique_ptr<Sprite> baseCounterSprite_ = nullptr;
    
    // スコアのカウンタースプライト用
    std::vector<std::unique_ptr<Sprite>> CounterSprite_;
    
    Vector2 counterOffset_ = { -60.0f, 5.0f }; // 右上基準からの数字のオフセット位置
    float counterSpacing_ = kDefaultCounterSpacing;
    float counterSize_ = kDefaultCounterSize;
    float ScoreSpriteWidth_ = kDefaultScoreSpriteWidth;
    int32_t currentScore ;
};