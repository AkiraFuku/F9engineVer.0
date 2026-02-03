#include "Player.h"
#include "ModelManager.h"
#include "Input.h"
#include "MoveEffect.h"
#include "DriftEffect.h"
#include "RotateArrow.h"
#include "SpeedMeter.h"
#include <GameEngine.h>
#include <Framework.h>
#include <cassert>

Player::Player() = default;

void Player::Initialize(Object3d* model, Camera* camera, const Vector3& position)
{
    // nullポインタチェック
    assert(model);
    assert(camera);

    // 引数をメンバ変数に記録
    model_ = model;
    camera_ = camera;
    cameraTransform_.rotate = camera->GetRotate();
    cameraTransform_.translate = camera->GetTranslate();
    camera_->Update();
    transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
    transform_.translate = position;
    worldMatrix_ = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    model_->SetTranslate(transform_.translate);
    model_->Update();

    // 速度を初期化
    velocity_ = { 0.0f, 0.0f, 0.2f };
    speedZ_ = velocity_.z;

    // 移動エフェクトの生成と初期化
    for (int i = 0; i < 3; ++i)
    {
        // モデルを生成してリストに追加
        auto model = std::make_unique<Object3d>();
        model->SetModel("moveEffect.obj");
        model->Initialize();
        moveEffectModels_.push_back(std::move(model));
        // エフェクト本体を生成してリストに追加
        auto effect = std::make_unique<MoveEffect>();
        effect->Initialize(moveEffectModels_.back().get(), camera_, transform_.translate * static_cast<float>(i + 2), this);
        moveEffects_.push_back(std::move(effect));
    }

    // 加速エフェクトの生成と初期化
    driftEffectSprite_ = std::make_unique<Sprite>();
    driftEffectSprite_->Initialize("resources/speedUp.png");
    driftEffectSprite_->SetPosition(Vector2{ 0.0f,0.0f });
    driftEffectSprite_->SetAnchorPoint(Vector2{ 0.0f,0.0f });
    driftEffect_ = std::make_unique<DriftEffect>();
    driftEffect_->Initialize(driftEffectSprite_.get(), camera_, transform_.translate, this);

    // 方向矢印の生成と初期化
    rotateArrowModel_ = std::make_unique<Object3d>();
    rotateArrowModel_->SetModel("rotateArrow.obj");
    rotateArrowModel_->Initialize();
    rotateArrow_ = std::make_unique<RotateArrow>();
    rotateArrow_->Initialize(rotateArrowModel_.get(), camera_, transform_.translate, this);

    // スピードメーターの生成と初期化
    baseSprite_ = std::make_unique<Sprite>();
    baseSprite_->Initialize("resources/base.png");
    baseSprite_->SetPosition(Vector2{ 800.0f, 250.0f });
    baseSprite_->SetAnchorPoint(Vector2{ 0.0f, 0.0f });
    speedMeterSprite_ = std::make_unique<Sprite>();
    speedMeterSprite_->Initialize("resources/speedMeter.png");
    speedMeterSprite_->SetPosition(Vector2{1250.0f, 800.0f });
    speedMeterSprite_->SetAnchorPoint(Vector2{ 1.0f, 1.0f });
    speedMeter_ = std::make_unique<SpeedMeter>();
    speedMeter_->Initialize(speedMeterSprite_.get(), baseSprite_.get(), camera_, this);

}

void Player::Update(bool isGameStarted)
{

    isGameStarted_ = isGameStarted;

    if (isGoal_)
    {
        // ゴールしたら動かない
        return;
    }

    if (isDead_)
    {
      transform_.rotate =
          Lerp(transform_.rotate, deadRotate_, kInterpolationRate);

      // タイマーを減らす
      if (deathTimer_ > 0.0f) {
        deathTimer_ -= 1.0f;
      }
    }

    else
    {

        if (isGameStarted_)
        {
            // 速度が0になったら
            if (speedZ_ <= 0.001f)
            {
                // 死亡させる
                speedZ_ = 0.0f;
                isDead_ = true;
                gameScene_->SetGameOver(true);
                return;
            }

            // 方向操作
            Rotate();

            if (isDriftStart_)
            {
                // ドリフト中の処理
                Drift();
            }
            else
            {
                if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE))
                {
                    // スペースキーを押したらドリフト開始
                    isDriftStart_ = true;
                    // 開始時の速度を記録
                    preSpeedZ_ = speedZ_;
                    // 開始時の角度を記録
                    angleY_ = transform_.rotate.y;
                }
            }


            // 移動処理
            Move();

        }
        // カメラの更新
        MoveCamera();
        camera_->Update();

        // 移動エフェクトの更新
        for (auto& effect : moveEffects_)
        {
            effect->Update();
        }

        // 加速エフェクトの更新
        driftEffect_->Update();

        // 方向矢印の更新
        rotateArrow_->Update();

        // スピードメーターの更新
        speedMeter_->Update();

    }

    // モデルの更新
    model_->SetTranslate(transform_.translate);
    model_->SetRotate(transform_.rotate);
    model_->Update();
}

void Player::Draw()
{
    // プレイヤーの描画
    model_->Draw();

    if (!isDead_)
    {

        if (!isGameStarted_)
        {
            return;
        }

        // 移動エフェクトの描画
        for (auto& effect : moveEffects_)
        {
            effect->Draw();
        }

        if (isAcceleration_)
        {
            // 加速エフェクトの描画
            driftEffect_->Draw();
        }

        // 方向矢印の描画
        rotateArrow_->Draw();

        // スピードメーターの描画
        speedMeter_->Draw();
    }

}

void Player::Rotate()
{
    // 回転速さ[ラジアン/frame]
    const float kRotSpeed = 0.008f;

    // 押した方向で移動ベクトルを変更
    if (Input::GetInstance()->PushedKeyDown(DIK_A)) {
        transform_.rotate.y -= kRotSpeed;
    }
    else if (Input::GetInstance()->PushedKeyDown(DIK_D)) {
        transform_.rotate.y += kRotSpeed;
    }
}

void Player::Move()
{
    // 速度を初期化
    velocity_ = { 0.0f, 0.0f, speedZ_ };

    // 行列を更新
    worldMatrix_ = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // 速度を決定
    velocity_ = TransformNormal(velocity_, worldMatrix_);

    // 位置に速度を加算
    transform_.translate += velocity_;

}

void Player::MoveCamera()
{
    // 行列を更新
    worldMatrix_ = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // カメラの位置を更新
    Vector3 offset = { 0.0f, 10.0f, -20.0f };
    offset = TransformNormal(offset, worldMatrix_);
    targetPos_ = transform_.translate + offset;
    cameraTransform_.translate = Lerp(cameraTransform_.translate, targetPos_, kInterpolationRate);
    camera_->SetTranslate(cameraTransform_.translate);
    cameraTransform_.rotate.y = transform_.rotate.y;
    camera_->SetRotate(cameraTransform_.rotate);
}

void Player::Drift()
{
    if (isAcceleration_)
    {
        if (speedZ_ >= topSpeedZ_)
        {
            // 最大速度に達したら加速終了
            isAcceleration_ = false;
            isDriftStart_ = false;
            driftTimer_ = 0.0f;
            // 加速エフェクトのアルファ値をリセット
            driftEffect_->ResetAlpha();
        }
        else
        {
            speedZ_ += kAcceleration;
            // 速度の段階を決定
            DetermineSpeedStage();

        }

    }
    else
    {
        if (Input::GetInstance()->TriggerKeyUp(DIK_SPACE))
        {
            // どれくらい曲がったかを記録
            driftAngle_ = std::abs(transform_.rotate.y - angleY_);

            // 曲がった角度が小さかったら
            if (driftAngle_ < std::abs(decelerationAngle_))
            {
                // 減速
                speedZ_ *= kSpeedDecayRate;
                // ドリフト終了フラグを立てる
                isDriftStart_ = false;
            }
            else
            {
                // 最大速度を決定
                topSpeedZ_ = preSpeedZ_ + (driftAngle_ * driftTimer_);
                // 加速開始フラグを立てる
                isAcceleration_ = true;
            }

            if (topSpeedZ_ > kMaxSpeedZ)
            {
                // 最大速度を越さないよう調整
                topSpeedZ_ = kMaxSpeedZ;
            }

        }
        else
        {
            // 減速
            speedZ_ *= kSpeedDecayRate;
            // タイマーを加算
            driftTimer_ += 0.005f;

            // 速度の段階を決定
            DetermineSpeedStage();
        }

    }

}

void Player::DetermineSpeedStage()
{
    if (speedZ_ < kSlowSpeedZ)
    {
        currentSpeedStage_ = SpeedStage::kSlow;
    }
    else if (speedZ_ < kNormalSpeedZ)
    {
        currentSpeedStage_ = SpeedStage::kNormal;
    }
    else if (speedZ_ < kFastSpeedZ)
    {
        currentSpeedStage_ = SpeedStage::kFast;
    }
    else
    {
        currentSpeedStage_ = SpeedStage::kMax;
    }
}

Vector3 Player::GetWorldPosition()
{
    // ワールド座標を入れる変数
    Vector3 worldPos;
    // ワールド行列の平行移動成分を取得
    worldPos = transform_.translate;
    return worldPos;
}

AABB Player::GetAABB()
{
    Vector3 worldPos = GetWorldPosition();
    AABB aabb;

    aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
    aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };

    return aabb;
}

void Player::OnCollision(const ObstacleSlow* obstacleSlow)
{
    // スコア加算処理
}

void Player::OnCollision(const ObstacleNormal* obstacleNormal)
{
    // プレイヤーの速度が一定以下なら
    if (currentSpeedStage_ < SpeedStage::kNormal)
    {
        isDead_ = true;
        gameScene_->SetGameOver(true);
    }
}

void Player::OnCollision(const ObstacleFast* obstacleFast)
{
    // プレイヤーの速度が一定以下なら
    if (currentSpeedStage_ < SpeedStage::kFast)
    {
        isDead_ = true;
        gameScene_->SetGameOver(true);
    }
}

void Player::OnCollision(const ObstacleMax* obstacleMax)
{
    // プレイヤーの速度が一定以下なら
    if (currentSpeedStage_ < SpeedStage::kMax)
    {
        isDead_ = true;
        gameScene_->SetGameOver(true);
    }
}

void Player::OnCollision(const Goal* goal)
{
    if (isGoal_)
    {
        return;
    }

    isGoal_ = true;
    gameScene_->SetCleared(true);
}

void Player::OnCollision(const CourseWall* courseWall)
{
    isDead_ = true;
    gameScene_->SetGameOver(true);
}


Player::~Player()
{
    // 移動エフェクト用のメモリ解放
    moveEffects_.clear();
    // 加速エフェクト用のメモリ解放
    driftEffect_.reset();
    // 方向矢印用のメモリ解放
    rotateArrow_.reset();
    // スピードメーター用のメモリ解放
    speedMeter_.reset();
}
