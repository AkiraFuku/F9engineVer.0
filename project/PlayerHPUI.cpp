#include "PlayerHPUI.h"
#include "Player.h"
#include "Sprite.h"
void PlayerHPUI::Initialize( Player* player)
{
    if (!player)
    {
        return;
    }
    player_ = player;
    hpSprite_ = std::make_unique<Sprite>();
    hpSprite_->Initialize("resources/UI/HP.png");
    hpSprite_->SetAnchorPoint({ 0.0f, 0.0f });  
    hpSprite_->SetPosition({ 0.0f, 0.0f });
    hpSprite_->SetSize({ 200.0f, 20.0f });
    // 

    // プレイヤーのHPに応じてカウンタースプライトを作成

    // プレイヤーのHPが0より大きい場合のみカウンタースプライトを作成


    if (player_->GetHitPoints() > 0 )
    {

        for (int i = 0; i < player_->GetHitPoints(); i++)
        {
            // HPのカウンタースプライトを作成
            auto counterSprite = std::make_unique<Sprite>();
            counterSprite->Initialize("resources/UI/HP_Counter.png");
            counterSprite->SetPosition({ 0.0f + i * 20.0f, 0.0f });
            counterSprite->SetSize({ 20.0f, 20.0f });
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
    // HPのカウンタースプライトの数をプレイヤーのHPに合わせて調整
    int currentHP = player_->GetHitPoints();
    // HPが減った場合は余分なカウンタースプライトを削除
    while (CounterSprite_.size() > currentHP)
    {
        CounterSprite_.pop_back();
    }
    // HPが増えた場合は新しいカウンタースプライトを追加
    while (CounterSprite_.size() < currentHP)
    {
        auto counterSprite = std::make_unique<Sprite>();
        counterSprite->Initialize("resources/UI/HP_Counter.png");
        counterSprite->SetPosition({ 0.0f + CounterSprite_.size() * 20.0f, 0.0f });
        counterSprite->SetSize({ 20.0f, 20.0f });
        CounterSprite_.push_back(std::move(counterSprite));
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
