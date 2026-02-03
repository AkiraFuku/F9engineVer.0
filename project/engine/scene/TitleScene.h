#pragma once
#include "Audio.h"
#include "Camera.h"
#include "MathFunction.h"
#include "Model.h"
#include "Object3D.h"
#include "ParicleEmitter.h"
#include "Sprite.h"
#include "TextureManager.h"

#include "Fade.h"

#include "Scene.h"
#include <memory>
class TitleScene : public Scene {
public:
  enum class Phase {
    kIdle,
    kFadeIn, // フェードイン
    kFadOut, // フェードアウト
  };

  Phase phase_ = Phase::kIdle;

public:
  void Initialize() override;
  void Finalize() override;
  void Update() override;
  void Draw() override;

private:
  std::unique_ptr<Camera> camera;
  std::unique_ptr<Sprite> sprite;
  std::unique_ptr<Sprite> titleLogoSprite;
  std::unique_ptr<Sprite> pressStartSprite;

  // タイトルロゴを動かす用の変数
  float amplitude_ = 4.0f;
  float theta_ = 0.0f;

  uint32_t bgmHandle_ = 0;
  uint32_t enterHandle_ = 0;

  // メンバ
  std::unique_ptr<Fade> fade_;
  bool requestSceneChange_ = false;
};
