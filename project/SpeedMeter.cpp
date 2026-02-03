#include "SpeedMeter.h"
#include "Player.h"
#include "ModelManager.h"
#include <GameEngine.h>
#include <Framework.h>
#include <cassert>

void SpeedMeter::Initialize(Sprite* sprite, Sprite* baseSprite, Camera* camera,Player* player_)
{
    // nullポインタチェック
    assert(sprite);
    assert(camera);

    // 引数をメンバ変数に記録
    sprite_ = sprite;
    baseSprite_ = baseSprite;
    camera_ = camera;
    player = player_;

    sprite_->Update();
    baseSprite_->Update();
}

void SpeedMeter::Update()
{
    // プレイヤーの速度に応じて角度を変更
    float speedZ = player->GetSpeedZ();
    // 現在の角度を記録
    float rotation = sprite_->GetRotation();
    // 速度に応じて回転角度を計算
    rotation =  + (speedZ * 1.3f);
    sprite_->SetRotation(rotation);

    sprite_->Update();
    baseSprite_->Update();
}

void SpeedMeter::Draw()
{
    baseSprite_->Draw();
    sprite_->Draw();
}

SpeedMeter::~SpeedMeter()
{
}
