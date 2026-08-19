#include "StartPhase.h"
#include "GameScene.h"
#include "PlayPhase.h"
#include "Fade.h"
#include "RailPath.h"
#include "GoalObject.h"

void StartPhase::Initialize(Scene* scene)
{
    // フェードインを開始 (黒 -> 透明)
    Fade::GetInstance()->StartFadeIn(kFadeDuration_);

    GameScene* gameScene = static_cast<GameScene*>(scene);


}

void StartPhase::Update(Scene* scene)
{
    GameScene* gameScene = static_cast<GameScene*>(scene);

    if (gameScene->GetGoal()) {
        gameScene->GetGoal()->Update();
    }
    if (gameScene->GetStageRaill()) {
        gameScene->GetStageRaill()->Update();
    }

    //// 1. プレイヤーとエネミーのレール・高さトランスフォームを更新
    //if (gameScene->GetPlayer()) {
    //    gameScene->GetPlayer()->UpdateTransform();
    //}
    //for (auto& enemy : gameScene->GetEnemies()) {
    //    if (enemy) {
    //        enemy->UpdateTransform();
    //    }
    //}

    // 2. カメラコントローラーを更新してプレイヤーを追尾・注視
    if (gameScene->GetCamera()) {
        gameScene->GetCamera()->Update();
    }

    // 3. 最新のカメラ行列を反映してモデルの WVP 行列を更新
    if (gameScene->GetPlayer()) {
        gameScene->GetPlayer()->UpdateTransform();
    }
    for (auto& enemy : gameScene->GetEnemies()) {
        if (enemy) {
            enemy->UpdateTransform();
        }
    }

    // フェードインが完了したら PlayPhase に遷移する
    if (!Fade::GetInstance()->IsFading()) {
        gameScene->ChangePhase(std::make_unique<PlayPhase>());
    }
}

void StartPhase::Draw(Scene* scene)
{
    // フェードは SceneManager::Draw で一括描画されます
}

void StartPhase::Finalize(Scene* scene)
{
    // 終了処理があれば記載
}