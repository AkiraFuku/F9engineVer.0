#include "PlayerState.h"

#include "Input.h"
#include "Player.h"

#include <memory>

// ======================================
// 待機状態
// ======================================

// 初期化
void PlayerStateIdle::Initialize(Player* player)
{
}

// 更新
void PlayerStateIdle::Update(Player* player)
{
    // WASDで移動
    if (Input::GetInstance()->PushedKeyDown(DIK_W) || Input::GetInstance()->PushedKeyDown(DIK_S) ||
        Input::GetInstance()->PushedKeyDown(DIK_A) || Input::GetInstance()->PushedKeyDown(DIK_D)) {
        player->ChangeState(std::make_unique<PlayerStateMove>());

        return;
    }

    // SPACEで糸を発射
    if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {
        player->ChangeState(std::make_unique<PlayerStateShoot>());

        return;
    }
}

// ======================================
// 移動状態
// ======================================

// 初期化
void PlayerStateMove::Initialize(Player* player)
{
}

// 更新
void PlayerStateMove::Update(Player* player)
{
    Vector3 moveDirection = {};

    // 移動処理
    player->UpdateMove(moveDirection);

    // 入力がなくなったら待機状態に遷移
    if (moveDirection.x == 0.0f && moveDirection.z == 0.0f) {
        player->ChangeState(std::make_unique<PlayerStateIdle>());

        return;
    }

    // 移動中にSPACEを押して発射状態に遷移
    if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {
        player->ChangeState(std::make_unique<PlayerStateShoot>());

        return;
    }

}

// ======================================
// 発射状態
// ======================================

// 初期化
void PlayerStateShoot::Initialize(Player* player)
{
    frameCount_ = 0;

    // 状態に入った瞬間に一度だけ糸を発射
    player->FireThread();
}

// 更新
void PlayerStateShoot::Update(Player* player)
{
    // フレームカウントを加算
    frameCount_ += kDeltaTime;

    // 0.5秒で待機状態に遷移
    if (frameCount_ >= 0.5f) {
        player->ChangeState(std::make_unique<PlayerStateIdle>());
    }
}