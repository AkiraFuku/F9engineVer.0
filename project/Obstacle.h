#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include <Audio.h>
#include <memory>

class Player;
class Score;

class Obstacle
{
protected:
    // ワールド変換データ
    Transform transform_;

    // モデル
    Object3d* model_ = nullptr;

    // カメラ
    Camera* camera_ = nullptr;

    // 当たり判定サイズ
    float width;
    float height;

    // 死亡フラグ
    bool isDead_ = false;

    // スコア
    std::vector<std::unique_ptr<Score>> scores_;
    std::vector<std::unique_ptr<Object3d>> scoreModels_;
    static inline const uint32_t kNumScores = 5;
    int scoreValue_ = 10;

    // プレイヤー
    Player* player_;

    // SE
    uint32_t deathHandle_ = 0;

public:


    Obstacle();
    ~Obstacle();
    virtual void Initialize(Object3d* model, Camera* camera, const Vector3& position, Player* player) = 0;
    virtual void Update();
    virtual void Draw();
    // ワールド座標の取得
    virtual Vector3 GetWorldPosition() const;
    // AABBを取得
    virtual AABB GetAABB() const;
    // 衝突応答
    virtual void OnCollision(const Player* player) = 0;
    // 当たり判定
    bool isCollision(const AABB& aabb1, const AABB& aabb2);
    void CheckCollision();
    // スコア管理
    bool IsScoreNone() const;
};

