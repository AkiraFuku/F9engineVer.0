#pragma once
#include "Vector2.h"
#include <memory>
#include <vector>
class Player;
class Sprite;

enum HPUIState
{
    ACTIVE,// HP UIがアクティブな状態
    INACTIVE,// HP UIが非アクティブな状態
};

/// <summary>
/// プレイヤー形態マークの種類
/// </summary>
enum class StateMarkType
{
    Walk,   // 通常時  : WarlkMark
    Jump,   // バウンドロボット形態 : JumpMark
    Bullet, // テストロボット形態  : BulletMark
};

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


    //セッター
    void SetPosition(const Vector2& position) {
        position_ = position;
    }
    
    void SetCounterOffset(const Vector2& offset) {
        counterOffset_ = offset;
    }
    void SetCounterSpacing(float spacing) {
        counterSpacing_ = spacing;
    }
    void SetCounterSize(float size) {
        counterSize_ = size;
    }
    void SetHPSpriteWidth(float width) {
        hpSpriteWidth_ = width;
    }
    // ゲッター
    const Vector2& GetPosition() const {
        return position_;
    }

    const Vector2& GetCounterOffset() const {
        return counterOffset_;
    }
    float GetCounterSpacing() const {
        return counterSpacing_;
    }
    float GetCounterSize() const {
        return counterSize_;
    }
    float GetHPSpriteWidth() const {
        return hpSpriteWidth_;
    }

private:

    Vector2 position_ = { 15.0f, 10.0f }; // HP UIの位置
    float scale_ = 2.0f; // HP UIのスケール



    //プレイヤー
    Player* player_ = nullptr;
    //HPの背景スプライト用
    std::unique_ptr<Sprite> hpSprite_ = nullptr;
    //プレイヤーの形態状態の把握用スプライト（Markアイコン表示）
    std::unique_ptr<Sprite> stateSprite_ = nullptr;

    // Markテクスチャのインデックス
    size_t markTexIndexWalk_   = 0; // WarlkMark
    size_t markTexIndexJump_   = 1; // JumpMark
    size_t markTexIndexBullet_ = 2; // BulletMark

    // ライフカウンター右端からMarkアイコンまでのオフセット
    Vector2 markOffset_ = { 15.0f, 0.0f };
    // Markアイコンのサイズ
    float markSize_ = 32.0f;

    //HPのカウンター用スプライト
    std::unique_ptr<Sprite> baseCounterSprite_ = nullptr;
    std::vector<std::unique_ptr<Sprite>> CounterSprite_ ;
    // 追加：テクスチャリスト内のインデックスを保持
    size_t texIndexFull_ = ACTIVE;  // HPが有る時のインデックス
    size_t texIndexEmpty_ = INACTIVE; // HPが空の時のインデックス

    Vector2 counterOffset_ = { 5.0f, 0.5f }; // カウンタースプライトのオフセット位置
    

    float counterSpacing_ = 20.0f; // カウンタースプライト間のスペース
    float counterSize_ = 20.0f;    // カウンタースプライトのサイズ
    float hpSpriteWidth_ = 200.0f; // HP UIの幅

};

