#pragma once
#include "Sprite.h"
#include "DXCommon.h"
#include "MathFunction.h" // Lerpを使用するためにインクルード[cite: 2]
#include <memory>

class Fade {
public:
    enum class Status {
        None,   // フェードしていない
        FadeIn, // フェードイン中 (1 -> 0)
        FadeOut // フェードアウト中 (0 -> 1)
    };
    static Fade* GetInstance();
    Fade(const Fade&) = delete;
    Fade& operator=(const Fade&) = delete;
    friend struct std::default_delete<Fade>;

    void Initialize();
    //
    void StartFadeIn(float duration);
    void StartFadeOut(float duration);

    void Update();
    void Draw();

    // フェード中かどうかを取得
    bool IsFading() const {
       return fading;
    }
    // フェード状態を取得
    Status GetStatus() const {
        return status_;
    }

private:
    Fade() = default;
    ~Fade() = default;
    static std::unique_ptr<Fade> instance;

    std::unique_ptr<Sprite> fadeSprite_;

    const float kDeltaTime_ = DXCommon::kDeltaTime; // デルタタイム[cite: 3]
    bool fading=false;
    Status status_ = Status::None;
    float alpha_ = 0.0f;
    float fadeDuration_ = 1.0f; // フェードにかける時間（秒）[cite: 3]
    float timer_ = 0.0f;        // 経過時間[cite: 3]
};