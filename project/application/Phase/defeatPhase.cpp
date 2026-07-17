#include "defeatPhase.h"
#include "Sprite.h"
#include "WinApp.h"
void defeatPhase::Initialize(Scene* scene)
{
    Sprite_ = std::make_unique<Sprite>();
    Sprite_->Initialize("resources/uvChecker.png");
    Sprite_->SetPosition(WinApp::GetInstance()->GetWindowCenter());
    Sprite_->SetAnchorPoint({ 0.5f, 0.5f });
}

void defeatPhase::Update(Scene* scene)
{
    Sprite_->Update();
}

void defeatPhase::Draw(Scene* scene)
{
    Sprite_->Draw();
}

void defeatPhase::Finalize(Scene* scene)
{

       Sprite_.reset();
}
