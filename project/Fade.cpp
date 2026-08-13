#include "Fade.h"
#include "WinApp.h"
#include <algorithm> // std::clamp 用



std::unique_ptr<Fade> Fade::instance = nullptr;
Fade* Fade::GetInstance() {
    if (instance == nullptr) {
        struct Helper : public Fade {
            Helper() : Fade() {}
        };
        instance = std::make_unique<Helper>();
    }
    return instance.get();
}


void Fade::Initialize()
{
    fadeSprite_ = std::make_unique<Sprite>();
    fadeSprite_->Initialize("resources/human/white.png");
    fadeSprite_->SetSize(Vector2(WinApp::kClientWidth, WinApp::kClientHeight));
    fadeSprite_->SetAnchorPoint(Anchor::TopLeft);
    fadeSprite_->SetPosition(Vector2(0.0f, 0.0f));

    // 黒の透過表示を行うため通常ブレンドに変更を推奨（加算指定から変更）[cite: 4]
    fadeSprite_->SetBlendMode(BlendMode::Normal);

    status_ = Status::None;
    alpha_ = 1.0f;
    fadeSprite_->SetColor(Vector4(1.0f, 1.0f, 1.0f, alpha_));
}

void Fade::StartFadeIn(float duration)
{
    if (IsFading()) return;

    status_ = Status::FadeIn;
    fadeDuration_ = duration;
    timer_ = 0.0f;
    alpha_ = 1.0f; // フェードイン開始時は完全不透明（画面を真っ黒にする）
    fading = true;

}

void Fade::StartFadeOut(float duration)
{
    if (IsFading()) return;

    status_ = Status::FadeOut;
    fadeDuration_ = duration;
    timer_ = 0.0f;
    alpha_ = 0.0f; // フェードアウト開始時は透明
    fading = true;

}

void Fade::Update()
{
    if (status_ == Status::None)
    {
        return;
    }

    // 経過時間を進める
    timer_ += kDeltaTime_;

    // 進捗率 t (0.0 ～ 1.0) を算出
    float t = timer_ / fadeDuration_;
    t = std::clamp(t, 0.0f, 1.0f); // 1.0を超えないようにクランプ

    // 線形補間（Lerp）を用いてアルファ値を計算[cite: 1, 2]
    if (status_ == Status::FadeIn)
    {
        // 1.0 (黒) -> 0.0 (透明)
        alpha_ = Lerp(1.0f, 0.0f, t);
    } else if (status_ == Status::FadeOut)
    {
        // 0.0 (透明) -> 1.0 (黒)
        alpha_ = Lerp(0.0f, 1.0f, t);
    }

    // 時間が終了したらフェード完了
    if (t >= 1.0f)
    {
        status_ = Status::None;
        fading=false;
    }

    // アルファ値をスプライトに適用[cite: 4]
    fadeSprite_->SetColor(Vector4(1.0f, 1.0f, 1.0f, alpha_));
    fadeSprite_->Update();
}

void Fade::Draw()
{
    // フェード中、または完全黒で画面を隠している場合は描画する
    if (status_ == Status::None && alpha_ <= 0.0f)
    {
        return;
    }
    fadeSprite_->Draw();
}