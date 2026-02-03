#include "TutorialScene.h"

// 各種インクルード
#include "CourseWall.h"
#include "Goal.h"
#include "Input.h"
#include "LightManager.h"
#include "MapChipField.h"
#include "ModelManager.h"
#include "ObstacleFast.h"
#include "ObstacleMax.h"
#include "ObstacleNormal.h"
#include "ObstacleSlow.h"
#include "Player.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "imgui.h"

#include <algorithm> // for remove_if
#include <numbers>

// コンストラクタ
TutorialScene::TutorialScene() = default;

// デストラクタ
TutorialScene::~TutorialScene() {
  // 明示的なクリアは unique_ptr なので必須ではないが、
  // 依存関係（PlayerがObstacleを持つなど）がある場合は順序制御のために有効
  player_.reset();
  playerModel_.reset();

  obstacleSlow_.clear();
  obstacleNormal_.clear();
  obstacleFast_.clear();
  obstacleMax_.clear();

  // モデルも一括クリア
  obstacleModels_.clear();
  wallModels_.clear();
  goalModels_.clear();
}

void TutorialScene::Initialize() {
  // --- カメラ ---
  camera_ = std::make_unique<Camera>();
  camera_->SetRotate({0.3f, 0.0f, 0.0f});
  camera_->SetTranslate({0.0f, 0.0f, -5.0f});
  Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());
  ParticleManager::GetInstance()->Setcamera(camera_.get());

  Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());
  ParticleManager::GetInstance()->Setcamera(camera_.get());

  bgmHandle_ =  Audio::GetInstance()->LoadAudio("resources/sounds/tutorial.wav");
  enterHandle_ = Audio::GetInstance()->LoadAudio("resources/sounds/enter.wav");
  selectHandle_ = Audio::GetInstance()->LoadAudio("resources/sounds/cursor.wav");
  Audio::GetInstance()->PlayAudio(bgmHandle_, true);

  // --- ライト ---
  LightManager::GetInstance()->AddDirectionalLight({1, 1, 1, 1}, {0, -1, 0},
                                                   1.0f);

  // --- パーティクル・背景等 ---
  TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
  ParticleManager::GetInstance()->CreateParticleGroup(
      "Test", "resources/uvChecker.png");

  bgSprite_ = std::make_unique<Sprite>();
  bgSprite_->Initialize("resources/uvChecker.png");
  bgSprite_->SetPosition(Vector2{125.0f, 100.0f});
  bgSprite_->SetBlendMode(BlendMode::Add);
  bgSprite_->SetAnchorPoint(Vector2{0.5f, 0.5f});

  // --- 3Dオブジェクト初期化 ---
  // デバッグ用オブジェクト
  debugAxis_ = std::make_unique<Object3d>();
  debugAxis_->Initialize();
  debugPlane_ = std::make_unique<Object3d>();
  debugPlane_->Initialize();

  ModelManager::GetInstance()->LoadModel("plane.obj");
  ModelManager::GetInstance()->LoadModel("axis.obj");
  ModelManager::GetInstance()->CreateSphereModel("MySphere", 16);

  debugAxis_->SetTranslate(Vector3{0.0f, 10.0f, 0.0f});
  debugAxis_->SetModel("axis.obj");
  debugPlane_->SetModel("plane.obj");
  debugPlane_->SetBlendMode(BlendMode::Add);

  // 背景モデル
  backgroundModel_ = std::make_unique<Object3d>();
  backgroundModel_->SetModel("field.obj");
  backgroundModel_->Initialize();
  backgroundModel_->SetTranslate(Vector3{200.0f, 0.0f, 1400.0f});
  backgroundModel_->SetRadius(2000.0f);

  Transform emitterTrans = {
      {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  emitter_ =
      std::make_unique<ParicleEmitter>("Test", emitterTrans, 10, 5.0f, 0.0f);

  // --- マップチップ ---
  mapChipField_ = std::make_unique<MapChipField>();
  mapChipField_->LoadMapChipCsv("resources/stage2.csv");
  GenerateFieldObjects();

  // --- チュートリアル状態初期化 ---
  currentPhase_ = TutorialPhase::kMovement;
  destroyedObstaclesCount_ = 0;
  isPhaseCleared_ = false;
  waitTimer_ = 0;
  isPhaseActive_ = false;

  // --- UIスプライト初期化 ---
  auto CreateSprite = [&](const std::string &path, const Vector2 &size,
                          const Vector2 &pos, bool center = true) {
    auto s = std::make_unique<Sprite>();
    s->Initialize(path);
    s->SetAnchorPoint(center ? Vector2{0.5f, 0.5f} : Vector2{0.5f, 0.0f});
    s->SetSize(size);
    s->SetPosition(pos);
    return s;
  };

  startGuideSprite_ = CreateSprite("resources/tutorial/spaceToStart.png",
                                   {600.0f, 60.0f}, {640.0f, 340.0f});
  moveText_ = CreateSprite("resources/tutorial/move.png", {460.0f, 160.0f},
                           {640.0f, 100.0f}, false);
  clearedText_ = CreateSprite("resources/tutorial/cleared.png", {460.0f, 64.0f},
                              {640.0f, 100.0f}, false);
  obstacleExplanationSprite_ =
      CreateSprite("resources/tutorial/ObstacleDescription.png",
                   {640.0f, 500.0f}, {640.0f, 320.0f});
  destroyNumText_ = CreateSprite("resources/tutorial/destroyNum.png",
                                 {460.0f, 160.0f}, {640.0f, 100.0f}, false);
  driftExplanationSprite_ =
      CreateSprite("resources/tutorial/driftExplanation.png", {640.0f, 280.0f},
                   {640.0f, 250.0f});
  driftText_ = CreateSprite("resources/tutorial/drift.png", {400.0f, 160.0f},
                            {640.0f, 100.0f}, false);
  meterImpactText_ = CreateSprite("resources/tutorial/meterImpact.png",
                                  {300.0f, 210.0f}, {155.0f, 585.0f});
  tutorialClearedText_ = CreateSprite("resources/tutorial/tutorialCleared.png",
                                      {460.0f, 160.0f}, {640.0f, 130.0f});

  stageSelectText_ = CreateSprite("resources/tutorial/stageSelect.png",
                                  {460.0f, 64.0f}, {640.0f, 300.0f});
  retryText_ = CreateSprite("resources/tutorial/retry.png", {460.0f, 64.0f},
                            {640.0f, 380.0f});
  retryText_->SetColor({1.0f, 1.0f, 1.0f, alpha_});
  backTitleText_ = CreateSprite("resources/tutorial/backTitle.png",
                                {460.0f, 64.0f}, {640.0f, 460.0f});
  backTitleText_->SetColor({1.0f, 1.0f, 1.0f, alpha_});

  // ビットマップフォント
  bitmappedFontSprites_.reserve(10); // メモリ確保
  bitmappedFontSprites_.clear();
  for (int i = 0; i < 10; i++) {
    std::string path = "resources/numbers/" + std::to_string(i) + ".png";
    TextureManager::GetInstance()->LoadTexture(path);

    auto s = std::make_unique<Sprite>();
    s->Initialize(path);
    s->SetSize({1.0f, 1.0f});
    s->SetAnchorPoint({0.5f, 0.5f});
    bitmappedFontSprites_.push_back(std::move(s));
  }

  scoreDisplay_ = std::make_unique<Bitmappedfont>();
  scoreDisplay_->Initialize(&bitmappedFontSprites_, camera_.get());
  scoreDisplay_->SetNumber(0);
  scoreDisplay_->SetPosition({540.0f, 170.0f});
  scoreDisplay_->SetSize({60.0f, 60.0f});

  targetDisplay_ = std::make_unique<Bitmappedfont>();
  targetDisplay_->Initialize(&bitmappedFontSprites_, camera_.get());
  targetDisplay_->SetNumber(0);
  targetDisplay_->SetPosition({680.0f, 170.0f});
  targetDisplay_->SetSize({60.0f, 60.0f});

  // --- フェード ---
  fade_ = std::make_unique<Fade>();
  fade_->Initialize(camera_.get());
  sceneState_ = SceneState::kFadeIn;
  fade_->Start(Fade::Phase::kFadeIn);
}

void TutorialScene::Finalize() {
  LightManager::GetInstance()->ClearLights();
  ParticleManager::GetInstance()->ReleaseParticleGroup("Test");
}

void TutorialScene::Update() {
  if (fade_)
    fade_->Update();
  camera_->Update();

  // 共通の背景オブジェクト更新
  if (debugPlane_)
    debugPlane_->Update();
  if (debugAxis_)
    debugAxis_->Update();
  if (backgroundModel_)
    backgroundModel_->Update();

  switch (sceneState_) {
  case SceneState::kFadeIn:
    UpdatePausedState(false);
    if (fade_->IsFinished()) {
      sceneState_ = SceneState::kMain;
    }
    break;

  case SceneState::kMain:
    UpdateMainState();
    break;

  case SceneState::kFadeOut:
    UpdatePausedState(false);
    if (fade_->IsFinished()) {
      switch (currentOption_) {
      case CompleteOption::kRetry:
        ResetTutorialState();
        currentPhase_ = TutorialPhase::kMovement;
        sceneState_ = SceneState::kFadeIn;
        fade_->SetCamera(camera_.get());
        fade_->Start(Fade::Phase::kFadeIn);
        break;
      case CompleteOption::kGoToSelect:
        GetSceneManager()->ChangeScene("SelectScene");
        Audio::GetInstance()->StopAudio(bgmHandle_);
        break;
      case CompleteOption::kBackToTitle:
        GetSceneManager()->ChangeScene("TitleScene");
        Audio::GetInstance()->StopAudio(bgmHandle_);
        break;
      }
    }
    break;
  }

  UpdateDebugGUI();
}

void TutorialScene::UpdateMainState() {
  bool isCompletePhase = (currentPhase_ == TutorialPhase::kComplete);

  // 開始入力待ち
  if (!isPhaseActive_ && !isCompletePhase) {
    if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE) ||
        Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {
      isPhaseActive_ = true;
    }
  }

  // ゲームロジック or 停止中の更新
  if (isPhaseActive_ && !isCompletePhase) {
    UpdateGameLogic();
  } else {
    UpdatePausedState(isCompletePhase);
  }

  if (emitter_)
    emitter_->Update();
  UpdateUI();
}

void TutorialScene::UpdateGameLogic() {
  if (player_)
    player_->Update(true);

  UpdateAllObstacles();
  for (const auto &wall : walls_)
    wall->Update();
  for (const auto &goal : goals_)
    goal->Update();

  CheckAllCollisions();

  // 死亡判定
  if (player_ && player_->IsDead()) {
    if (player_->IsDeathAnimationFinished()) {
      ResetTutorialState();
    }
    return; // 死んだら以降の処理スキップ
  }

  UpdateTutorialSteps();
  RemoveDeadObstacles();
}

void TutorialScene::UpdatePausedState(bool isCompletePhase) {
  if (player_) {
    player_->Update(false); // 移動なし更新
    if (playerModel_)
      playerModel_->Update(); // モデル行列更新
  }

  // 停止中も描画のために行列更新は必要
  UpdateAllObstacles();
  for (const auto &wall : walls_)
    wall->Update();
  for (const auto &goal : goals_)
    goal->Update();

  if (isCompletePhase) {
    UpdateTutorialSteps();
  }
}

// 障害物一括更新（ループ展開）
void TutorialScene::UpdateAllObstacles() {
  for (const auto &obj : obstacleSlow_)
    obj->Update();
  for (const auto &obj : obstacleNormal_)
    obj->Update();
  for (const auto &obj : obstacleFast_)
    obj->Update();
  for (const auto &obj : obstacleMax_)
    obj->Update();
}

// テンプレートによる削除ロジックの共通化
template <typename T>
void TutorialScene::RemoveDeadFromList(std::vector<std::unique_ptr<T>> &list,
                                       bool countScore) {
  if (list.empty())
    return;

  size_t prevSize = list.size();
  std::erase_if(list, [](const auto &obj) { return obj->IsScoreNone(); });

  if (countScore) {
    destroyedObstaclesCount_ += static_cast<int>(prevSize - list.size());
  }
}

void TutorialScene::RemoveDeadObstacles() {
  bool isAttackPhase = (currentPhase_ == TutorialPhase::kAttack);

  RemoveDeadFromList(obstacleSlow_, isAttackPhase);
  RemoveDeadFromList(obstacleNormal_, isAttackPhase);
  RemoveDeadFromList(obstacleFast_, isAttackPhase);
  RemoveDeadFromList(obstacleMax_, isAttackPhase);
}

// ====================================================================
// UI更新の最適化: 表示するものだけUpdateを呼ぶ
// ====================================================================
void TutorialScene::UpdateUI() {
  // 常に必要な背景スプライトの更新
  if (bgSprite_)
    bgSprite_->Update();

  // フェーズクリア時の「Cleared」表示を最優先
  if (isPhaseCleared_) {
    if (clearedText_)
      clearedText_->Update();
    return; // クリア表示中は他のUI更新をスキップ
  }

  // フェーズごとのUI更新
  switch (currentPhase_) {
  case TutorialPhase::kMovement:
    if (!isPhaseActive_) {
      // 入力待ち：SPACEガイドを表示
      if (startGuideSprite_)
        startGuideSprite_->Update();
    } else {
      // プレイ中：移動操作ガイドを表示
      if (moveText_)
        moveText_->Update();
    }
    break;

  case TutorialPhase::kDrift:
    if (!isPhaseActive_) {
      // 入力待ち：ドリフト説明とSPACEガイドを表示
      if (driftExplanationSprite_)
        driftExplanationSprite_->Update();
      if (startGuideSprite_)
        startGuideSprite_->SetPosition({640.0f, 450.0f});
        startGuideSprite_->Update();
    } else {
      // プレイ中：ドリフト中のテキストを表示
      if (driftText_)
        driftText_->Update();
    }
    break;

  case TutorialPhase::kAttack:
    if (!isPhaseActive_) {
      // 入力待ち：障害物破壊説明とSPACEガイドを表示
      if (obstacleExplanationSprite_)
        startGuideSprite_->SetPosition({640.0f, 600.0f});
        obstacleExplanationSprite_->Update();
      if (startGuideSprite_)
        startGuideSprite_->Update();
    } else {
      // プレイ中：破壊数やインパクトのUIを表示
      if (destroyNumText_)
        destroyNumText_->Update();
      if (meterImpactText_)
        meterImpactText_->Update();

      // スコア表示の更新
      int displayNum = std::min(destroyedObstaclesCount_, 9);
      if (scoreDisplay_) {
        scoreDisplay_->SetNumber(displayNum);
        scoreDisplay_->Update();
      }
      if (targetDisplay_) {
        targetDisplay_->SetNumber(kTargetDestroyCount_);
        targetDisplay_->Update();
      }
    }
    break;

  case TutorialPhase::kComplete:
    // リザルト画面のUI更新
    if (tutorialClearedText_)
      tutorialClearedText_->Update();
    if (retryText_)
      retryText_->Update();
    if (stageSelectText_)
      stageSelectText_->Update();
    if (backTitleText_)
      backTitleText_->Update();
    break;
  }
}

void TutorialScene::Draw() {
  // --- 3D描画セクション ---
  if (debugPlane_)
    debugPlane_->Draw();
  if (debugAxis_)
    debugAxis_->Draw();
  ParticleManager::GetInstance()->Draw();
  if (backgroundModel_)
    backgroundModel_->Draw();

  if (player_)
    player_->Draw();

  DrawAllObstacles();

  for (const auto &wall : walls_)
    wall->Draw();
  for (const auto &goal : goals_)
    goal->Draw();

  // --- 2D UI描画セクション ---
  if (isPhaseCleared_) {
    if (clearedText_)
      clearedText_->Draw();
  } else {
    switch (currentPhase_) {
    case TutorialPhase::kMovement:
      if (!isPhaseActive_) {
        if (startGuideSprite_)
          startGuideSprite_->Draw();
      } else {
        if (moveText_)
          moveText_->Draw();
      }
      break;

    case TutorialPhase::kDrift:
      if (!isPhaseActive_) {
        if (driftExplanationSprite_)
          driftExplanationSprite_->Draw();
        if (startGuideSprite_)
          startGuideSprite_->Draw();
      } else {
        if (driftText_)
          driftText_->Draw();
      }
      break;

    case TutorialPhase::kAttack:
      if (!isPhaseActive_) {
        if (obstacleExplanationSprite_)
          obstacleExplanationSprite_->Draw();
        if (startGuideSprite_)
          startGuideSprite_->Draw();
      } else {
        if (destroyNumText_)
          destroyNumText_->Draw();
        if (meterImpactText_)
          meterImpactText_->Draw();
        if (scoreDisplay_)
          scoreDisplay_->Draw();
        if (targetDisplay_)
          targetDisplay_->Draw();
      }
      break;

    case TutorialPhase::kComplete:
      if (tutorialClearedText_)
        tutorialClearedText_->Draw();
      if (retryText_)
        retryText_->Draw();
      if (stageSelectText_)
        stageSelectText_->Draw();
      if (backTitleText_)
        backTitleText_->Draw();
      break;
    }
  }

  // フェードの描画（最前面）
  if (fade_)
    fade_->Draw();
}

void TutorialScene::DrawAllObstacles() {
  for (const auto &o : obstacleSlow_)
    o->Draw();
  for (const auto &o : obstacleNormal_)
    o->Draw();
  for (const auto &o : obstacleFast_)
    o->Draw();
  for (const auto &o : obstacleMax_)
    o->Draw();
}

void TutorialScene::CheckAllCollisions() {
  if (!isPhaseActive_ || !player_)
    return;

  AABB aabbPlayer = player_->GetAABB();

  // 衝突チェックヘルパー
  auto CheckList = [&](auto &list) {
    for (auto &obj : list) {
      if (isCollision(aabbPlayer, obj->GetAABB())) {
        player_->OnCollision(obj.get());
        obj->OnCollision(player_.get());
      }
    }
  };

  CheckList(obstacleSlow_);
  CheckList(obstacleNormal_);
  CheckList(obstacleFast_);
  CheckList(obstacleMax_);

  for (auto &wall : walls_) {
    if (isCollision(aabbPlayer, wall->GetAABB())) {
      player_->OnCollision(wall.get());
    }
  }
  for (auto &goal : goals_) {
    if (isCollision(aabbPlayer, goal->GetAABB())) {
      player_->OnCollision(goal.get());
    }
  }
}

bool TutorialScene::isCollision(const AABB &aabb1, const AABB &aabb2) {
  return (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x &&
          aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y &&
          aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z);
}

void TutorialScene::UpdateTutorialSteps() {
  // 1. クリア後の待機
  if (isPhaseCleared_) {
    if (--waitTimer_ <= 0) {
      isPhaseCleared_ = false;
      // 次のフェーズへ
      switch (currentPhase_) {
      case TutorialPhase::kMovement:
        currentPhase_ = TutorialPhase::kDrift;
        isPhaseActive_ = false;
        break;
      case TutorialPhase::kDrift:
        currentPhase_ = TutorialPhase::kAttack;
        destroyedObstaclesCount_ = 0;
        isPhaseActive_ = false;
        break;
      case TutorialPhase::kAttack:
        currentPhase_ = TutorialPhase::kComplete;
        isPhaseActive_ = false;
        break;
      default:
        break;
      }
    }
    return;
  }

  // 2. クリア条件チェック
  if (!player_)
    return;

  switch (currentPhase_) {
  case TutorialPhase::kMovement:
    if (Input::GetInstance()->PushedKeyDown(DIK_A) ||
        Input::GetInstance()->PushedKeyDown(DIK_D)) {
      isPhaseCleared_ = true;
      waitTimer_ = kPhaseWaitTime_;
    }
    break;
  case TutorialPhase::kDrift:
    if (player_->GetSpeedZ() >= kTargetDriftSpeed_) {
      isPhaseCleared_ = true;
      waitTimer_ = kPhaseWaitTime_;
    }
    break;
  case TutorialPhase::kAttack:
    if (destroyedObstaclesCount_ >= kTargetDestroyCount_) {
      isPhaseCleared_ = true;
      waitTimer_ = kPhaseWaitTime_;
    }
    break;
  case TutorialPhase::kComplete: { // スコープを明確にするための波カッコ
    bool isChanged = false;

    // --- メニュー操作 (上入力) ---
    if (Input::GetInstance()->TriggerKeyDown(DIK_W) ||
        Input::GetInstance()->TriggerKeyDown(DIK_UP)) {
      Audio::GetInstance()->PlayAudio(selectHandle_, false);
      isChanged = true;

      if (currentOption_ == CompleteOption::kGoToSelect) {
        currentOption_ = CompleteOption::kBackToTitle; // 一番上なら一番下へ
      } else if (currentOption_ == CompleteOption::kRetry) {
        currentOption_ = CompleteOption::kGoToSelect;
      } else if (currentOption_ == CompleteOption::kBackToTitle) {
        currentOption_ = CompleteOption::kRetry;
      }
    } // ← ここで上入力のifを閉じる

    // --- メニュー操作 (下入力) ---
    if (Input::GetInstance()->TriggerKeyDown(DIK_S) ||
        Input::GetInstance()->TriggerKeyDown(DIK_DOWN)) {
      Audio::GetInstance()->PlayAudio(selectHandle_, false);
      isChanged = true;

      if (currentOption_ == CompleteOption::kGoToSelect) {
        currentOption_ = CompleteOption::kRetry;
      } else if (currentOption_ == CompleteOption::kRetry) {
        currentOption_ = CompleteOption::kBackToTitle;
      } else if (currentOption_ == CompleteOption::kBackToTitle) {
        currentOption_ = CompleteOption::kGoToSelect; // 一番下なら一番上へ
      }
    }

    // --- 色更新 ---
    // 入力があったとき、または毎フレーム更新
    Vector4 sel = {1.0f, 1.0f, 1.0f, 1.0f};
    Vector4 unsel = {1.0f, 1.0f, 1.0f, alpha_};

    stageSelectText_->SetColor(
        currentOption_ == CompleteOption::kGoToSelect ? sel : unsel);
    retryText_->SetColor(currentOption_ == CompleteOption::kRetry ? sel
                                                                  : unsel);
    backTitleText_->SetColor(
        currentOption_ == CompleteOption::kBackToTitle ? sel : unsel);

    // --- 決定 ---
    if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A) ||
        Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {
      sceneState_ = SceneState::kFadeOut;
      fade_->Start(Fade::Phase::kFadeOut);
      Audio::GetInstance()->PlayAudio(enterHandle_);
    }
  } break;
  }
}

void TutorialScene::ResetTutorialState() {
  player_.reset();
  playerModel_.reset();

  obstacleSlow_.clear();
  obstacleNormal_.clear();
  obstacleFast_.clear();
  obstacleMax_.clear();
  obstacleModels_.clear(); // 所有権を持つモデルもクリア

  walls_.clear();
  wallModels_.clear();
  goals_.clear();
  goalModels_.clear();

  isPhaseCleared_ = false;
  waitTimer_ = 0;
  destroyedObstaclesCount_ = 0;

  GenerateFieldObjects();
}

void TutorialScene::GenerateFieldObjects() {
  uint32_t numBlockVertical = mapChipField_->GetNumBlockVirtical();
  uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

  // ベクターのメモリ予約 (パフォーマンス向上)
  obstacleSlow_.reserve(10);
  obstacleNormal_.reserve(10);
  walls_.reserve(50);
  // ... 必要に応じて他も

  // ヘルパー関数: モデル生成とオブジェクト生成
  auto CreateObstacle = [&](auto &list, const char *modelName,
                            const Vector3 &pos, int type) {
    auto model = std::make_unique<Object3d>();
    model->SetModel(modelName);
    model->Initialize();

    using ObjType = typename std::remove_reference<
        decltype(*list.begin())>::type::element_type;
    auto obj = std::make_unique<ObjType>();
    obj->Initialize(model.get(), camera_.get(), pos, player_.get());

    obstacleModels_.push_back(std::move(model)); // モデルを一括管理用コンテナへ
    list.push_back(std::move(obj));
  };

  // --- Pass 1: Player生成 ---
  for (uint32_t i = 0; i < numBlockVertical; ++i) {
    for (uint32_t j = 0; j < numBlockHorizontal; j++) {
      if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kPlayer) {
        Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j, i);
        playerModel_ = std::make_unique<Object3d>();
        playerModel_->SetTranslate(pos);
        playerModel_->SetModel("maguro.obj");
        playerModel_->Initialize();
        player_ = std::make_unique<Player>();
        player_->Initialize(playerModel_.get(), camera_.get(), pos);
      }
    }
  }

  // --- Pass 2: その他オブジェクト ---
  for (uint32_t i = 0; i < numBlockVertical; ++i) {
    for (uint32_t j = 0; j < numBlockHorizontal; j++) {
      MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);
      Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j, i);

      switch (type) {
      case MapChipType::kObstacle:
        if (player_) {
          uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
          if (subID == 0)
            CreateObstacle(obstacleSlow_, "ika.obj", pos, 0);
          else if (subID == 1)
            CreateObstacle(obstacleNormal_, "taru.obj", pos, 1);
          else if (subID == 2)
            CreateObstacle(obstacleFast_, "shark.obj", pos, 2);
          else if (subID == 3)
            CreateObstacle(obstacleMax_, "ship.obj", pos, 3);
        }
        break;

      case MapChipType::kGoal: {
        auto gModel = std::make_unique<Object3d>();
        gModel->SetModel("goal.obj");
        gModel->Initialize();
        auto goal = std::make_unique<Goal>();
        goal->Initialize(gModel.get(), camera_.get(), pos);
        goalModels_.push_back(std::move(gModel));
        goals_.push_back(std::move(goal));
      } break;

      case MapChipType::kWallStraight: {
        auto wModel = std::make_unique<Object3d>();
        wModel->SetModel("wall.obj");
        wModel->Initialize();
        auto wall = std::make_unique<CourseWall>();
        wall->Initialize(wModel.get(), camera_.get(), pos);
        wall->SetScale({2.0f, 4.0f, 2.0f});
        wallModels_.push_back(std::move(wModel));
        walls_.push_back(std::move(wall));
      } break;

      case MapChipType::kWallTurn: {
        // ... カーブ壁の計算ロジック（変更なし）
        auto wModel = std::make_unique<Object3d>();
        wModel->SetModel("wall.obj");
        wModel->Initialize();

        // 位置調整ロジック
        Vector3 wallPos = pos;
        const float dx =
            (MapChipField::kBlockWidth - MapChipField::kTurnWidth) * 2.0f;
        const float dz =
            (MapChipField::kBlockHeight - MapChipField::kTurnHeight) * 2.0f;
        uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
        if (subID == 0) {
          wallPos.x += dx;
          wallPos.z += dz;
        } else if (subID == 1) {
          wallPos.x -= dx;
          wallPos.z += dz;
        } else if (subID == 2) {
          wallPos.x -= dx;
          wallPos.z -= dz;
        } else if (subID == 3) {
          wallPos.x += dx;
          wallPos.z -= dz;
        }

        auto wall = std::make_unique<CourseWall>();
        wall->Initialize(wModel.get(), camera_.get(), wallPos);
        wall->SetScale({2.0f, 4.0f, 2.0f});
        wallModels_.push_back(std::move(wModel));
        walls_.push_back(std::move(wall));
      } break;
      default:
        break;
      }
    }
  }
}

void TutorialScene::UpdateDebugGUI() {
#ifdef USE_IMGUI
  ImGui::Begin("Tutorial Status");
  if (isPhaseCleared_) {
    ImGui::TextColored({0, 1, 0, 1}, "Phase Cleared! Wait: %d", waitTimer_);
  } else {
    const char *phaseNames[] = {"Movement", "Drift", "Attack", "Complete"};
    ImGui::Text("Phase: %s", phaseNames[(int)currentPhase_]);
  }
  ImGui::End();
#endif
}