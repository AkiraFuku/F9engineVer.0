#pragma once

#include <memory>
#include <string>

#include "Scene.h"
#include "Sprite.h"
#include <Audio.h>

class Fade;
class Camera;

class SelectScene : public Scene {
public:
  enum class Selection {
    kTutorial, // チュートリアル
    kStage1,
    kStage2,
    kStage3,
    kNone,     // 選択なし
  };

  Selection selection_ = Selection::kNone;

public:
  void Initialize() override;
  void Finalize() override;
  void Update() override;
  void Draw() override;

private:
  int currentSelectionIndex_ = 0;

  // 次に遷移するシーン名を保存しておく変数
  std::string nextSceneName_ = "";

  std::unique_ptr<Camera> camera_;
  std::unique_ptr<Fade> fade_;

  std::unique_ptr<Sprite> backGround_;
  std::unique_ptr<Sprite> selectCursor_;
  std::unique_ptr<Sprite> tutorialSprite_;
  std::unique_ptr<Sprite> stage1Sprite_;
  std::unique_ptr<Sprite> stage2Sprite_;
  std::unique_ptr<Sprite> stage3Sprite_;


  uint32_t bgmHandle_ = 0;
  uint32_t enterHandle_ = 0;
  uint32_t selectHandle_ = 0;

  bool requestSceneChange_ = false;
};
