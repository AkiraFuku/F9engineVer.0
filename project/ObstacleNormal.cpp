#include "ObstacleNormal.h"
#include "Player.h"
#include "Score.h"
#include <cassert>

void ObstacleNormal::Initialize(Object3d* model, Camera* camera, const Vector3& position, Player* player)
{
    // nullポインタチェック
    assert(model);
    assert(camera);

    // 引数をメンバ変数に記録
    model_ = model;
    camera_ = camera;
    transform_.translate = position;
    player_ = player;

    // 当たり判定サイズを調整
    width = 2.0f;
    height = 2.0f;

    // スコア値を設定
    scoreValue_ = 40;

    // モデルの更新
    model_->SetTranslate(transform_.translate);
    model_->Update();
}

void ObstacleNormal::OnCollision(const Player* player)
{
    // プレイヤーの速度が一定以上なら破壊
    if (player->GetSpeedStage() >= SpeedStage::kNormal)
    {
        if (isDead_) return;
        // スコアの生成と初期化
        for (int i = 0; i < kNumScores; i++)
        {
            auto model = std::make_unique<Object3d>();
            model->SetModel("normalScore.obj");
            model->Initialize();
            scoreModels_.push_back(std::move(model));
            auto score = std::make_unique<Score>();
            score->Initialize(scoreModels_.back().get(), camera_, transform_.translate, const_cast<Player*>(player), scoreValue_);
            scores_.push_back(std::move(score));
        }
        isDead_ = true;
    }
}
