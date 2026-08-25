#include "GaidUI.h"
#include "Player.h"
#include "Sprite.h"

void GaidUI::Initialize(Player* player)
{
    player_ = player;

    // スプライト生成・初期化の共通処理用ラムダ
    auto CreateGuideSprite = [this](const std::string& texturePath) -> std::unique_ptr<Sprite> {
        auto sprite = std::make_unique<Sprite>();
        sprite->Initialize(texturePath);
        sprite->SetAnchorPoint(Anchor::TopLeft);
        sprite->SetSize(guideSize_);
        return sprite;
    };

    // 1. 各スプライトの生成
    moveSprite_   = CreateGuideSprite("resources/Gaid/MoveGaid.png");
    attackSprite_ = CreateGuideSprite("resources/Gaid/AtackGaid.png");
    jumpSprite_   = CreateGuideSprite("resources/Gaid/JumpGaid.png");
    shootSprite_  = CreateGuideSprite("resources/Gaid/ShootGaid.png");
    aimSprite_    = CreateGuideSprite("resources/Gaid/AIMGaid.png");
}

void GaidUI::Update()
{
    if (!player_) return;

    // プレイヤーの現在の状態（Behavior）を確認
    std::string behaviorName = player_->GetBehaviorName() ? player_->GetBehaviorName() : "";
    isAimMode_ = (behaviorName == "Aim");

    if (isAimMode_) {
        // --- AIM（射撃待機）中: Aim と Shoot を表示 ---
        if (aimSprite_) {
            aimSprite_->SetPosition({ basePosition_.x, basePosition_.y });
            aimSprite_->SetSize(guideSize_);
            aimSprite_->Update();
        }
        if (shootSprite_) {
            shootSprite_->SetPosition({ basePosition_.x + spacing_, basePosition_.y });
            shootSprite_->SetSize(guideSize_);
            shootSprite_->Update();
        }
    }
    else {
        // --- 通常時: Move, Attack, Jump, Shoot を並べて表示 ---
        int index = 0;
        if (moveSprite_) {
            moveSprite_->SetPosition({ basePosition_.x + spacing_ * (index++), basePosition_.y });
            moveSprite_->SetSize(guideSize_);
            moveSprite_->Update();
        }
        if (attackSprite_) {
            attackSprite_->SetPosition({ basePosition_.x + spacing_ * (index++), basePosition_.y });
            attackSprite_->SetSize(guideSize_);
            attackSprite_->Update();
        }
        if (jumpSprite_) {
            jumpSprite_->SetPosition({ basePosition_.x + spacing_ * (index++), basePosition_.y });
            jumpSprite_->SetSize(guideSize_);
            jumpSprite_->Update();
        }
        if (shootSprite_) {
            shootSprite_->SetPosition({ basePosition_.x + spacing_ * (index++), basePosition_.y });
            shootSprite_->SetSize(guideSize_);
            shootSprite_->Update();
        }
    }
}

void GaidUI::Draw()
{
    if (!player_) return;

    if (isAimMode_) {
        // AIM時のみ描画
        if (aimSprite_) aimSprite_->Draw();
        if (shootSprite_) shootSprite_->Draw();
    }
    else {
        // 通常時描画
        if (moveSprite_) moveSprite_->Draw();
        if (attackSprite_) attackSprite_->Draw();
        if (jumpSprite_) jumpSprite_->Draw();
        if (shootSprite_) shootSprite_->Draw();
    }
}
