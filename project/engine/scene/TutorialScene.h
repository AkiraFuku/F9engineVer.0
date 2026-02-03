#pragma once
#include "Scene.h"

// エンジン・ユーティリティ
#include "Audio.h"
#include "Bitmappedfont.h"
#include "Camera.h"
#include "Fade.h"
#include "ParicleEmitter.h"
#include "Sprite.h"

// ゲームオブジェクト（前方宣言）
class Player;
class ObstacleSlow;
class ObstacleNormal;
class ObstacleFast;
class ObstacleMax;
class MapChipField;
class Goal;
class CourseWall;
class Object3d; // Object3dも前方宣言で十分な場合はこちらへ

#include <memory>
#include <vector>

class TutorialScene : public Scene {
public:
  TutorialScene();
  ~TutorialScene() override;

  void Initialize() override;
  void Finalize() override;
  void Update() override;
  void Draw() override;

private:
  // --- メイン処理 ---
  void UpdateMainState();
  void UpdateGameLogic();
  void UpdatePausedState(bool isCompletePhase);

  // --- UI & デバッグ ---
  void UpdateUI(); // アクティブなUIのみ更新するよう最適化
  void UpdateDebugGUI();

  // --- オブジェクト管理 ---
  void UpdateAllObstacles();
  void DrawAllObstacles();
  void RemoveDeadObstacles();

  // --- 判定 & 進行 ---
  void CheckAllCollisions();
  bool isCollision(const AABB &aabb1, const AABB &aabb2);
  void UpdateTutorialSteps();

  // --- 生成 & リセット ---
  void GenerateFieldObjects();
  void ResetTutorialState();

  // ヘルパー: 指定コンテナから死亡フラグのついたオブジェクトを削除
  template <typename T>
  void RemoveDeadFromList(std::vector<std::unique_ptr<T>> &list,
                          bool countScore);

private:
  // カメラ・基本コンポーネント
  std::unique_ptr<Camera> camera_; // 名前統一: camera -> camera_
  std::unique_ptr<Fade> fade_;
  Audio::SoundHandle handle_ = 0;

  // 背景・環境
  std::unique_ptr<Object3d> backgroundModel_;
  std::unique_ptr<Sprite> bgSprite_; // sprite -> bgSprite_ (命名明確化)
  std::unique_ptr<ParicleEmitter> emitter_;

  // プレイヤー
  std::unique_ptr<Player> player_;
  std::unique_ptr<Object3d> playerModel_;

  // 障害物
  // (Update/Drawを一括管理するため、必要なら基底クラスのポインタ管理も検討すべきだが、今回は既存構造を維持)
  std::vector<std::unique_ptr<ObstacleSlow>> obstacleSlow_;
  std::vector<std::unique_ptr<ObstacleNormal>> obstacleNormal_;
  std::vector<std::unique_ptr<ObstacleFast>> obstacleFast_;
  std::vector<std::unique_ptr<ObstacleMax>> obstacleMax_;

  // モデルキャッシュ (GenerateFieldObjectsで生成したモデルの所有権)
  std::vector<std::unique_ptr<Object3d>> obstacleModels_; // まとめて管理
  std::vector<std::unique_ptr<Object3d>> wallModels_;
  std::vector<std::unique_ptr<Object3d>> goalModels_;

  // 静的オブジェクト
  std::vector<std::unique_ptr<CourseWall>> walls_;
  std::vector<std::unique_ptr<Goal>> goals_;

  // マップ
  std::unique_ptr<MapChipField> mapChipField_;

  // --- 状態管理 ---
public:
  enum class SceneState { kFadeIn, kMain, kFadeOut };
  SceneState sceneState_ = SceneState::kFadeIn;

  enum class TutorialPhase { kMovement, kDrift, kAttack, kComplete };
  TutorialPhase currentPhase_ = TutorialPhase::kMovement;

  // ゲーム進行パラメータ
  bool isPhaseActive_ = false;
  bool isPhaseCleared_ = false;
  int waitTimer_ = 0;
  int destroyedObstaclesCount_ = 0;

  static constexpr int kTargetDestroyCount_ = 3;
  static constexpr float kTargetDriftSpeed_ = 0.3f;
  static constexpr int kPhaseWaitTime_ = 90;

  // --- UI関連 ---
  std::vector<std::unique_ptr<Sprite>> bitmappedFontSprites_;
  std::unique_ptr<Bitmappedfont> scoreDisplay_;
  std::unique_ptr<Bitmappedfont> targetDisplay_;

  // Spriteポインタ (命名整理)
  std::unique_ptr<Sprite> startGuideSprite_;
  std::unique_ptr<Sprite> moveText_;
  std::unique_ptr<Sprite> clearedText_;
  std::unique_ptr<Sprite> obstacleExplanationSprite_;
  std::unique_ptr<Sprite> destroyNumText_;
  std::unique_ptr<Sprite> driftExplanationSprite_;
  std::unique_ptr<Sprite> driftText_;
  std::unique_ptr<Sprite> meterImpactText_;
  std::unique_ptr<Sprite> tutorialClearedText_;

  // Complete画面用
  std::unique_ptr<Sprite> stageSelectText_;
  std::unique_ptr<Sprite> retryText_;
  std::unique_ptr<Sprite> backTitleText_;

  float alpha_ = 0.3f;

  enum class CompleteOption { kRetry, kGoToSelect, kBackToTitle };
  CompleteOption currentOption_ = CompleteOption::kGoToSelect;

  // デバッグ用オブジェクト (必要に応じて残す)
  std::unique_ptr<Object3d> debugPlane_;
  std::unique_ptr<Object3d> debugAxis_;
};