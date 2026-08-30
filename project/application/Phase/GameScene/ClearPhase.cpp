#include "ClearPhase.h"
#include "Sprite.h"
#include "WinApp.h"
#include "SceneManager.h"
#include "Input.h"
#include "Fade.h"
void ClearPhase::Initialize(Scene* scene)
{

    clearSprite_ = std::make_unique<Sprite>();
    clearSprite_->Initialize("resources/StageClear/tekutekuClear.png");

    clearSprite_->SetPosition(WinApp::GetInstance()->GetWindowCenter());
    clearSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    ClearSE = Audio::GetInstance()->LoadAudio("resources/Audio/BGM/Stageclear.mp3");
    Play_ = Audio::GetInstance()->PlayAudio(ClearSE, false, 0.25f, "SE");

}

void ClearPhase::Update(Scene* scene)
{
    clearSprite_->Update();

    if (!isTransitioning_) {
        if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {
            Fade::GetInstance()->StartFadeOut(1.0f); // 1.0秒かけてフェードアウト
            isTransitioning_ = true;
        }
    } else {
        if (!Fade::GetInstance()->IsFading()) {
            SceneManager::GetInstance()->ChangeScene("TitleScene");
        }
    }
    if (!Audio::GetInstance()->IsPlaying(Play_))
    {
        if (!Audio::GetInstance()->IsPlaying(scene->getBGMPlayHundle())) {
            Audio::GetInstance()->ResumeAudio(scene->getBGMPlayHundle());
        }
    }



}

void ClearPhase::Draw(Scene* scene)
{
    clearSprite_->Draw();
}

void ClearPhase::Finalize(Scene* scene)
{
    clearSprite_.reset();
}
