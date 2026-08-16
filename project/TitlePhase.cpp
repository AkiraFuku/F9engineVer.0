#include "TitlePhase.h"
#include "Input.h"
#include "SceneManager.h"

#include "Fade.h"

void TitlePhase::Initialize(Scene* scene)
{}

void TitlePhase::Update(Scene* scene)
{


    //if (!Fade::GetInstance()->IsFading())
    //{



    if (!isTransitioning_) {
        if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {
            Fade::GetInstance()->StartFadeOut(2.0f); // 1.0秒かけてフェードアウト
            isTransitioning_ = true;
        }
    } else {
        if (!Fade::GetInstance()->IsFading()) {
            SceneManager::GetInstance()->ChangeScene("GameScene");
        }
    }

       // }
}

void TitlePhase::Draw(Scene* scene)
{}

void TitlePhase::Finalize(Scene* scene)
{}
