#include "ClearPhase.h"
#include "Sprite.h"
#include "WinApp.h"
void ClearPhase::Initialize(Scene* scene)
{

    clearSprite_ = std::make_unique<Sprite>();
    clearSprite_->Initialize("resources/uvChecker.png");

    clearSprite_->SetPosition(WinApp::GetInstance()->GetWindowCenter());
    clearSprite_->SetAnchorPoint({ 0.5f, 0.5f });



}

void ClearPhase::Update(Scene * scene)
{
    clearSprite_->Update();

}

void ClearPhase::Draw(Scene * scene)
{
    clearSprite_->Draw();
}

void ClearPhase::Finalize(Scene * scene)
{
//今後何か処理を実装する
}
