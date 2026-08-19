#include "PlayerHPUI.h"
#include "Player.h"
#include "Sprite.h"
void PlayerHPUI::Initialize(Player* player)
{
    if (!player)
    {
        return;
    }
    //HP UIの背景部分
    player_ = player;
    hpSprite_ = std::make_unique<Sprite>();
    hpSprite_->Initialize("resources/uvChecker.png");
    hpSprite_->SetAnchorPoint({ Anchor::TopLeft });
    hpSprite_->SetPosition({ position_.x, position_.y });
    hpSprite_->SetSize({ hpSpriteWidth_ * scale_, 20.0f * scale_ });
    // 
    

     baseCounterSprite_ = std::make_unique<Sprite>();
    // 1. 初期テクスチャ（実HP）で初期化
    baseCounterSprite_->Initialize("resources/HPUI/ActiveHP.png");

    // 2. 空テクスチャを追加登録
    baseCounterSprite_->RegisterTexture("resources//HPUI/DeactiveHP.png");

    // 座標とサイズの設定
    baseCounterSprite_->SetPosition({ position_.x + counterOffset_.x + (counterSpacing_+counterSize_ )* scale_, position_.y + counterOffset_.y });
    baseCounterSprite_->SetSize({ counterSize_ * scale_, counterSize_ * scale_ });
    baseCounterSprite_->SetAnchorPoint({ Anchor::TopLeft });// 中心を基準にする場合


    // プレイヤーの最大HPを取得 (※関数名は実際のPlayerクラスのものに合わせてください)
    int maxHP = player_->GetMaxHitPoints();

    if (maxHP > 0)
    {
        for (int i = 0; i < maxHP; i++)
        {
            // 2. 雛形からコピーを作成
            auto counterSprite = baseCounterSprite_->Clone();
            
            // 固有の座標だけ個別に設定して登録
            counterSprite->SetPosition({ position_.x + counterOffset_.x + i * (counterSpacing_+counterSize_) * scale_, position_.y + counterOffset_.y });
            CounterSprite_.push_back(std::move(counterSprite));
        }
    }

}

void PlayerHPUI::Update()
{
    if (!player_)
    {
        return;
    }
    int currentHP = player_->GetHitPoints();// 現在のHPを取得
    int maxHP = player_->GetMaxHitPoints();  // 最大のHPを取得
    // もしゲーム中に最大HPが上昇・下降した場合に備え、スプライト数を最大HPに同期させる
    while (CounterSprite_.size() > static_cast<size_t>(maxHP))
    {
        CounterSprite_.pop_back();
    }
    while (CounterSprite_.size() < static_cast<size_t>(maxHP))
    {
        auto counterSprite = baseCounterSprite_->Clone();
        counterSprite->SetPosition({ position_.x + counterOffset_.x + CounterSprite_.size() * counterSpacing_ * scale_, position_.y + counterOffset_.y });
        CounterSprite_.push_back(std::move(counterSprite));
    }

    // 各スプライトのテクスチャを現在HPに応じて切り替える
    for (int i = 0; i < CounterSprite_.size(); i++)
    {
        if (i < currentHP)
        {
            // 現在HP以下のインデックスは「実HP(インデックス0)」
            CounterSprite_[i]->SetTextureByIndex(texIndexFull_);
        } else
        {
            // 現在HPを超えたインデックスは「空HP(インデックス1)」
            CounterSprite_[i]->SetTextureByIndex(texIndexEmpty_);
        }
    }

    hpSprite_->Update(); 

    for (auto& counterSprite : CounterSprite_)
    {
        counterSprite->Update(); 
    }

}

void PlayerHPUI::Draw()
{
    if (!player_)
    {
        return;
    }

    hpSprite_->Draw();

    for (auto& counterSprite : CounterSprite_)
    {
        counterSprite->Draw();
    }
}
