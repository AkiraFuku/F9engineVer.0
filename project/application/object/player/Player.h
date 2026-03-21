#pragma once

#include "Camera.h"
#include "Model.h"
#include "Object3D.h"
#include "DrawFunction.h"
#include"PlayerState.h"

class CollisionMask;
class ThreadManager;

class Player {
public:
    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="pos">初期位置</param>
    /// <param name="threadManager">ThreadManagerのポインタ</param>
    void Initialize(const Vector3& pos, ThreadManager* thread);

    /// <summary>
    /// 終了
    /// </summary>
    void Finalize();

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

    /// <summary>
    /// 移動処理
    /// </summary>
    void UpdateMove(Vector3& moveDirection);

    /// <summary>
    /// 衝突判定
    /// </summary>
    void IsCollision();

    /// <summary>
    /// 移動距離確定
    /// </summary>
    void ResultMove();

    /// <summary>
    ///
    /// </summary>
    void IsCollisionSDF();

    /// <summary>
    /// 状態遷移
    /// </summary>
    /// <param name="newState">次の状態</param>
    void ChangeState(std::unique_ptr<IPlayerState> newState);

    /// <summary>
    /// 糸を発射する処理
    /// </summary>
    void FireThread();

public: // 外部入出力
    // ----- Getter -----

    // 位置
    Vector3 GetPosition() const { return object_->GetTranslate(); }
    void SetPosition(const Vector3& pos);

    // 向いている方向
    Vector3 GetForward()const;

    // 糸の上を歩いているか？
    bool OnThread()const { return onThread_; }

    // AABBを取得
    AABB GetAABB() const;

    // アフィン行列
    Matrix4x4 GetWorldMatrix() const;

private:
    // 現在の状態
    std::unique_ptr<IPlayerState> state_;

    // モデル
    std::unique_ptr<Object3d> object_;

    // 拡縮
    Vector3 scale_ = {1.0f, 1.0f, 1.0f};
    // 回転
    Vector3 rotate_ = {0.0f, 0.0f, 0.0f};
    // 平行移動
    Vector3 translate_ = {0.0f, 0.0f, 0.0f};

    // 現状向いているY軸の角度(単位: ラジアン)
    float rotationY_ = 0.0f;
    // 旋回速度
    static inline const float kTurnSpeed = 0.15f;

    // キャラクターの当たり判定サイズ
    static inline const float kWidth = 1.6f;
    static inline const float kHeight = 1.6f;

    // 速度
    Vector3 velocity_ = {0.1f, 0.0f, 0.1f};
    // 実際に動く時の速度
    Vector3 moveVel_;

    // ThreadManager
    ThreadManager* thread_ = nullptr;

    // 調整用パラメータ
    float threadInfluenceRadius_ = 1.5f; // 糸を巻き込む広さ
    float threadPlayerWeight_ = 0.05f;   // 糸を沈ませる重さ
    float threadWalkRadius_ = 1.0f;      // 糸に乗れる判定半径

    // 糸の上を歩いているかのフラグ
    bool onThread_ = false;

private:

    /// <summary>
    /// 糸の相互作用
    /// </summary>
    void UpdateThreadInteraction();
};
