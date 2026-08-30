#include "defeatPhase.h"
#include "Sprite.h"
#include "WinApp.h"
#include "SceneManager.h"
#include "Input.h"

#include "Fade.h"

void defeatPhase::Initialize(Scene* scene)
{
    Sprite_ = std::make_unique<Sprite>();
    Sprite_->Initialize("resources/GAMOVER/tekutekuGameOver.png");
    Sprite_->SetPosition(WinApp::GetInstance()->GetWindowCenter());
    Sprite_->SetAnchorPoint({ 0.5f, 0.5f });

    defeatSE = Audio::GetInstance()->LoadAudio("resources/Audio/BGM/GameOver.mp3");
    Play_ = Audio::GetInstance()->PlayAudio(defeatSE, false, 0.25f, "SE");


}

void defeatPhase::Update(Scene* scene)
{
    Sprite_->Update();

    if (!isTransitioning_) {
        if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {
            Fade::GetInstance()->StartFadeOut(1.0f); // 1.0秒かけてフェードアウト
            isTransitioning_ = true;
        }
    } else {
        if (!Fade::GetInstance()->IsFading()) {
            SceneManager::GetInstance()->ChangeScene("GameScene");
        }
    }
    if (!Audio::GetInstance()->IsPlaying(Play_)) {
        if (!Audio::GetInstance()->IsPlaying(scene->getBGMPlayHundle())) {
            Audio::GetInstance()->ResumeAudio(scene->getBGMPlayHundle());
        }
    }

}

void defeatPhase::Draw(Scene* scene)
{
    Sprite_->Draw();
}

void defeatPhase::Finalize(Scene* scene)
{

    Sprite_.reset();
}
