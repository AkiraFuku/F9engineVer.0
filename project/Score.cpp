#include "Score.h"
#include "Player.h"
#include "ModelManager.h"
#include <GameEngine.h>
#include <Framework.h>
#include <cassert>

void Score::Initialize(Object3d* model, Camera* camera, const Vector3& position, Player* player_,int score)
{
    // nullポインタチェック
    assert(model);
    assert(camera);

    // 引数をメンバ変数に記録
    model_ = model;
    camera_ = camera;
    transform_.translate = position;
    player = player_;
    score_ = score;

    // ランダム値を決定
    rand_ = new Rand();
    rand_->Initialize();
    rand_->RandomInitialize();
    randomValue = static_cast<int>(rand_->GetRandom());

    // 位置を設定
    transform_.translate.x +=  randomValue;
    transform_.translate.y += randomValue;
    transform_.translate.z +=  randomValue;
    model_->SetTranslate(transform_.translate);
    model_->Update();

    // SEの読み込み
    scoreHandle_ = Audio::GetInstance()->LoadAudio("resources/sounds/score.wav");
}

void Score::Update()
{
    if (!isDead_)
    {
        // 速度をリセット
        velocity_ = { 0.0f, 0.0f, 0.0f };

        // プレイヤーに向かって進む
        Vector3 direction = player->GetWorldPosition() - transform_.translate;
        float length = sqrtf(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
        if (length != 0.0f)
        {
            direction.x /= length;
            direction.y /= length;
            direction.z /= length;
        }
        velocity_ = direction * speed_; // スピード調整
        transform_.translate += velocity_;
        // モデルの更新
        model_->SetTranslate(transform_.translate);
        model_->Update();
    }
}

void Score::Draw()
{
    if (!isDead_)
    {
        model_->Draw();
    }
}

Vector3 Score::GetWorldPosition()
{
    // ワールド座標を入れる変数
    Vector3 worldPos;
    // ワールド行列の平行移動成分を取得
    worldPos = transform_.translate;
    return worldPos;
}

AABB Score::GetAABB()
{
    Vector3 worldPos = GetWorldPosition();
    AABB aabb;

    aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
    aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };

    return aabb;
}

void Score::OnCollision()
{
    if (isDead_) return;

    // スコアを加算
    player->AddScore(score_);
    // デスフラグを立てる
    isDead_ = true;
    // SE再生
    Audio::GetInstance()->PlayAudio(scoreHandle_, false);
}

Score::~Score()
{
}
