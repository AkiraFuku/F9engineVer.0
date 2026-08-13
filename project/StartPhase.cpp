#include "StartPhase.h"
#include "GameScene.h"
#include "PlayPhase.h"
#include "Fade.h"

void StartPhase::Initialize(Scene* scene)
{
    // フェードインを開始 (黒 -> 透明)
   // Fade::GetInstance()->StartFadeIn(kFadeDuration_);
}

void StartPhase::Update(Scene* scene)
{
    GameScene* gameScene = static_cast<GameScene*>(scene);

    // フェードの更新処理
   //Fade::GetInstance()->Update();

    // 背景オブジェクトやカメラなど、演出に必要な最小限の更新（必要に応じて）
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
    //// フェードの描画
   // Fade::GetInstance()->Draw();
}

void StartPhase::Finalize(Scene* scene)
{
    // 終了処理があれば記載
}