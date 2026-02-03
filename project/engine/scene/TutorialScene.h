#pragma once
#include "Audio.h"
#include "Camera.h"
#include "DrawFunction.h"
#include "MathFunction.h"
#include "Model.h"
#include "Object3D.h"
#include "ParicleEmitter.h"
#include "Scene.h"
#include "Sprite.h"
#include "TextureManager.h"
#include <memory>

class Player;
class ObstacleSlow;
class ObstacleNormal;
class ObstacleFast;
class ObstacleMax;
class MapChipField;
class Bitmappedfont;

class Score;
class Goal;

class CourseWall;

#include "Fade.h"

class TutorialScene : public Scene {
public:
  void Initialize() override;
  void Finalize() override;
  void Update() override;
  void Draw() override;
  TutorialScene();
  ~TutorialScene() override;

  // 全ての当たり判定を行う
  void CheckAllCollisions();
  // 当たり判定
  bool isCollision(const AABB &aabb1, const AABB &aabb2);

  // マップチップの生成
  void GenerateFieldObjects();

  // 開始フラグを取得
  bool IsStarted() const { return isStarted_; }

private:
  std::unique_ptr<Camera> camera;
  std::unique_ptr<Sprite> sprite;
  std::unique_ptr<Object3d> object3d2;
  std::unique_ptr<Object3d> object3d;
  std::unique_ptr<ParicleEmitter> emitter;
  
  // 自キャラ
  std::unique_ptr<Player> player_;
  // プレイヤーのモデル
  std::unique_ptr<Object3d> playerModel_;

  // 障害物
  std::vector<std::unique_ptr<ObstacleSlow>> obstacleSlow_;
  std::vector<std::unique_ptr<ObstacleNormal>> obstacleNormal_;
  std::vector<std::unique_ptr<ObstacleFast>> obstacleFast_;
  std::vector<std::unique_ptr<ObstacleMax>> obstacleMax_;

  // 障害物のモデル
  std::vector<std::unique_ptr<Object3d>> obstacleSlowModel_;
  std::vector<std::unique_ptr<Object3d>> obstacleNormalModel_;
  std::vector<std::unique_ptr<Object3d>> obstacleFastModel_;
  std::vector<std::unique_ptr<Object3d>> obstacleMaxModel_;

  // レースコースの壁
  std::vector<std::unique_ptr<Object3d>> wallModels_;
  std::vector<std::unique_ptr<CourseWall>> walls_;

  // ゴール
  std::vector<std::unique_ptr<Goal>> goals_;
  std::vector<std::unique_ptr<Object3d>> goalModels_;

  // スタート前のカウントダウン
  int32_t countdownTimer_ = 180;
  // ゲーム開始フラグ
  bool isStarted_ = true;

  // マップチップフィールド
  std::unique_ptr<MapChipField> mapChipField_;
  // ブロック用のワールドトランスフォーム
  std::vector<std::vector<Transform *>> worldTransformObjects;

  // メンバ
  std::unique_ptr<Fade> fade_;
  bool requestSceneChange_ = false;

public:
  enum class SceneState {
    kFadeIn, // フェードイン中（開始時・リセット時）
    kMain,   // メイン処理中（プレイ可能）
    kFadeOut // フェードアウト中（シーン遷移・リセット前）
  };

  SceneState sceneState_ = SceneState::kFadeIn;

public:
  enum class TutorialPhase {
    kMovement, // WASD移動
    kDrift,    // SPACE減速
    kAttack,   // 障害物破壊
    kComplete  // チュートリアル完了
  };

  TutorialPhase currentPhase_ = TutorialPhase::kMovement;
  int destroyedObstaclesCount_ = 0;   // 破壊した障害物の数
  const int kTargetDestroyCount_ = 3; // クリアに必要な破壊数
  const float kTargetDriftSpeed_ = 0.3f;

  bool isPhaseCleared_ = false; // クリア状態か？
  int waitTimer_ = 0;           // 待機タイマー
  const int kPhaseWaitTime_ = 90;

  // ★追加: チュートリアル進行更新用関数
  void UpdateTutorialSteps();

  void ResetTutorialState();

  // 数字用スプライトの配列（0～9などを格納）
  std::vector<std::unique_ptr<Sprite>> numberSprites_;
  // 数字表示管理クラス
  std::unique_ptr<Bitmappedfont> scoreDisplay_;

  // 目標数用のスプライト配列
  std::vector<std::unique_ptr<Sprite>> targetNumberSprites_;
  // 目標数表示管理クラス
  std::unique_ptr<Bitmappedfont> targetDisplay_;

  std::unique_ptr<Sprite> moveText_;
  std::unique_ptr<Sprite> backTitleText_;
  std::unique_ptr<Sprite> clearedText_;
  std::unique_ptr<Sprite> destroyNumText_;
  std::unique_ptr<Sprite> driftText_;
  std::unique_ptr<Sprite> meterImpactText_;
  std::unique_ptr<Sprite> retryText_;
  std::unique_ptr<Sprite> stageSelectText_;
  std::unique_ptr<Sprite> tutorialClearedText_;

  // BGM、SEの読み込み
  uint32_t bgmHandle_ = 0;
  uint32_t enterHandle_ = 0;
  uint32_t selectHandle_ = 0;

  float alpha_ = 0.3f;

public:
  enum class CompleteOption {
    kRetry,      // 最初からやり直す
    kGoToSelect, // ゲームシーンへ
    kBackToTitle // タイトルへ
  };

  CompleteOption currentOption_ = CompleteOption::kGoToSelect;
};