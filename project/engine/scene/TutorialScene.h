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
#include <vector>

// 前方宣言
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
  TutorialScene();
  ~TutorialScene() override;

  void Initialize() override;
  void Finalize() override;
  void Update() override;
  void Draw() override;

private:
  // --- リファクタリングで追加した内部処理用関数 ---

  // メイン状態（ゲームプレイ中）の更新処理
  void UpdateMainState();
  // ゲームロジック（移動・物理・判定）の更新
  void UpdateGameLogic();
  // 停止中（待機・完了画面）の描画用更新
  void UpdatePausedState(bool isCompletePhase);
  // UIの更新
  void UpdateUI();
  // デバッグUIの更新（ImGui）
  void UpdateDebugGUI();

  // 障害物リストを一括処理するためのヘルパー
  void UpdateAllObstacles();
  void DrawAllObstacles();
  void RemoveDeadObstacles();

public:
  // 全ての当たり判定を行う
  void CheckAllCollisions();
  // AABB同士の当たり判定
  bool isCollision(const AABB &aabb1, const AABB &aabb2);
  // マップチップの生成
  void GenerateFieldObjects();
  // チュートリアル進行更新
  void UpdateTutorialSteps();
  // リセット処理
  void ResetTutorialState();

private:
  std::unique_ptr<Camera> camera;
  std::unique_ptr<Sprite> sprite;
  std::unique_ptr<Object3d> object3d2;
  std::unique_ptr<Object3d> object3d;
  std::unique_ptr<ParicleEmitter> emitter;
  Audio::SoundHandle handle_;

  // 自キャラ
  std::unique_ptr<Player> player_;
  std::unique_ptr<Object3d> playerModel_;

  // 障害物とそのモデル
  std::vector<std::unique_ptr<ObstacleSlow>> obstacleSlow_;
  std::vector<std::unique_ptr<ObstacleNormal>> obstacleNormal_;
  std::vector<std::unique_ptr<ObstacleFast>> obstacleFast_;
  std::vector<std::unique_ptr<ObstacleMax>> obstacleMax_;

  std::vector<std::unique_ptr<Object3d>> obstacleSlowModel_;
  std::vector<std::unique_ptr<Object3d>> obstacleNormalModel_;
  std::vector<std::unique_ptr<Object3d>> obstacleFastModel_;
  std::vector<std::unique_ptr<Object3d>> obstacleMaxModel_;

  // 壁・ゴール
  std::vector<std::unique_ptr<Object3d>> wallModels_;
  std::vector<std::unique_ptr<CourseWall>> walls_;
  std::vector<std::unique_ptr<Goal>> goals_;
  std::vector<std::unique_ptr<Object3d>> goalModels_;

  // ゲーム進行フラグ
  bool isPhaseActive_ = false;

  std::unique_ptr<MapChipField> mapChipField_;
  std::vector<std::vector<Transform *>> worldTransformObjects;

  std::unique_ptr<Fade> fade_;

public:
  enum class SceneState { kFadeIn, kMain, kFadeOut };
  SceneState sceneState_ = SceneState::kFadeIn;

  enum class TutorialPhase { kMovement, kDrift, kAttack, kComplete };
  TutorialPhase currentPhase_ = TutorialPhase::kMovement;

  int destroyedObstaclesCount_ = 0;
  const int kTargetDestroyCount_ = 3;
  const float kTargetDriftSpeed_ = 0.3f;

  bool isPhaseCleared_ = false;
  int waitTimer_ = 0;
  const int kPhaseWaitTime_ = 90;

  // UI関連
  std::vector<std::unique_ptr<Sprite>> numberSprites_;
  std::unique_ptr<Bitmappedfont> scoreDisplay_;
  std::vector<std::unique_ptr<Sprite>> targetNumberSprites_;
  std::unique_ptr<Bitmappedfont> targetDisplay_;

  std::unique_ptr<Sprite> startGuideSprite_;
  std::unique_ptr<Sprite> clearedText_;
  std::unique_ptr<Sprite> moveText_;
  std::unique_ptr<Sprite> driftExplanationSprite_;
  std::unique_ptr<Sprite> driftText_;
  std::unique_ptr<Sprite> meterImpactText_;
  std::unique_ptr<Sprite> obstacleExplanationSprite_;
  std::unique_ptr<Sprite> destroyNumText_;
  std::unique_ptr<Sprite> stageSelectText_;
  std::unique_ptr<Sprite> retryText_;
  std::unique_ptr<Sprite> backTitleText_;
  std::unique_ptr<Sprite> tutorialClearedText_;

  float alpha_ = 0.3f;

  enum class CompleteOption { kRetry, kGoToSelect, kBackToTitle };
  CompleteOption currentOption_ = CompleteOption::kGoToSelect;
};