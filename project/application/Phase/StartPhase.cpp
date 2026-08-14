#include "StartPhase.h"
#include "GameScene.h"
#include "PlayPhase.h"
#include "Fade.h"
#include "RailPath.h"

void StartPhase::Initialize(Scene* scene)
{
    // フェードインを開始 (黒 -> 透明)
    Fade::GetInstance()->StartFadeIn(kFadeDuration_);

    GameScene* gameScene = static_cast<GameScene*>(scene);

    //gameScene->GetPlayer()->Update();
    //gameScene->GetPlayer()->Draw();
    //gameScene->Update();
    //gameScene->Draw();

    //const auto& enemies = gameScene->GetEnemies();
    //for (auto& enemy : enemies) {
    //    enemy->Update();
    //    enemy->Draw();
    //}
}

void StartPhase::Update(Scene* scene)
{
    GameScene* gameScene = static_cast<GameScene*>(scene);

    gameScene->GetStageRaill()->Update();

    // スタートフェーズ中はプレイヤーとエネミーを動かさない
    // （Update を呼ばず、位置は初期化時の状態を維持）

    // カメラコントローラーを更新してプレイヤーとエネミーを映す
    if (gameScene->GetCamera()) {
        gameScene->GetCamera()->Update();
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