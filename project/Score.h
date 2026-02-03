#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include "Rand.h"
#include <Audio.h>
#include <memory>

class Player;

class Score
{
private:
    // ワールド変換データ
    Transform transform_;

    // モデル
    Object3d* model_ = nullptr;

    // カメラ
    Camera* camera_ = nullptr;

    // 速度
    Vector3 velocity_ = {};
    float speed_ = 0.8f;

    // プレイヤーのポインタ
    Player* player = nullptr;

    // ランダム用ポインタ
    Rand* rand_ = nullptr;
    int randomValue = 0;

    // 保有スコア
    int score_ = 0;

    // デスフラグ
    bool isDead_ = false;

    // 当たり判定サイズ
    static inline float kWidth = 2.0f;
    static inline float kHeight = 2.0f;

    // SE
    uint32_t scoreHandle_ = 0;

public:
    // 初期化
    void Initialize(Object3d* model, Camera* camera, const Vector3& position, Player* player_,int score);
    // 更新
    void Update();
    // 描画
    void Draw();
    // ワールド座標の取得
    Vector3 GetWorldPosition();
    // AABBを取得
    AABB GetAABB();
    // 衝突応答
    void OnCollision();
    // デスフラグを取得
    bool IsDead() const { return isDead_; }
    // コンストラクタとデストラクタ
    Score() = default;
    ~Score();

};

