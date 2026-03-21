#pragma once

#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include "DrawFunction.h"

class Player;

class Egg
{
public:

    void Initialize();

    void Finalize();

    void Update();

    void Draw();

    // ワールド座標を取得
    Vector3 GetWorldPosition() const;

    // AABBを取得
    AABB GetAABB() const;

    // 衝突応答
    void OnCollision(const Player* player_);

    // ヒットフラグをセット
    void SetHitFlag(bool isHit) { isHit_ = isHit; }

    // プレイヤーのポインタ
    void SetPlayer(Player* player) { player_ = player; }

    // 卵の状況を取得
    bool IsOnPlayer() const { return onPlayer_; }

    // HPをセット
    void SetHP(float hp) { HP_ -= hp; }

    // 死亡判定
    void Death();

    
public: // 外部入出力

    

private:

    std::unique_ptr<Object3d> object_;

    Vector3 scale_ = { 1.0f,1.0f,1.0f };
    Vector3 rotate_ = { 0.0f,0.0f,0.0f };
    Vector3 translate_ = { 0.0f,0.0f,0.0f };

    // キャラクターの当たり判定サイズ
    static inline const float kWidth = 1.6f;
    static inline const float kHeight = 1.6f;

    // 当たり判定フラグ
    bool isHit_ = false;
    // 卵を持ち上げるフラグ
    bool onPlayer_ = false;
    // プレイヤーのポインタ
    Player* player_ = nullptr;

    // HP
    float HP_ = 10.0f;
};

