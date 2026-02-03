#include "TutorialScene.h"
#include "CourseWall.h"
#include "Input.h"

#include "Bitmappedfont.h"
#include "Goal.h"
#include "LightManager.h"
#include "MapChipField.h"
#include "ModelManager.h"
#include "ObstacleFast.h"
#include "ObstacleMax.h"
#include "ObstacleNormal.h"
#include "ObstacleSlow.h"
#include "PSOMnager.h"
#include "Player.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "imgui.h"

#include <algorithm> // for remove_if
#include <numbers>

// コンストラクタ
TutorialScene::TutorialScene() = default;

// デストラクタ
TutorialScene::~TutorialScene() {
  player_.reset();
  playerModel_.reset();
  // unique_ptrのvectorはclearで十分解放されるが明示的に
  obstacleSlow_.clear();
  obstacleSlowModel_.clear();
  obstacleNormal_.clear();
  obstacleNormalModel_.clear();
  obstacleFast_.clear();
  obstacleFastModel_.clear();
  obstacleMax_.clear();
  obstacleMaxModel_.clear();
  walls_.clear();
  wallModels_.clear();
  goals_.clear();
  goalModels_.clear();
}

void TutorialScene::Initialize() {
  // --- カメラ ---
  camera = std::make_unique<Camera>();
  camera->SetRotate({0.3f, 0.0f, 0.0f});
  camera->SetTranslate({0.0f, 0.0f, -5.0f});
  Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());
  ParticleManager::GetInstance()->Setcamera(camera.get());

  // --- オーディオ ---
  handle_ = Audio::GetInstance()->LoadAudio("resources/fanfare.mp3");
  Audio::GetInstance()->PlayAudio(handle_, true);

  // --- ライト ---
  LightManager::GetInstance()->AddDirectionalLight({1, 1, 1, 1}, {0, -1, 0},
                                                   1.0f);

  // --- パーティクル・背景等 ---
  TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
  ParticleManager::GetInstance()->CreateParticleGroup(
      "Test", "resources/uvChecker.png");

  sprite = std::make_unique<Sprite>();
  sprite->Initialize("resources/uvChecker.png");
  sprite->SetPosition(Vector2{125.0f, 100.0f});
  sprite->SetBlendMode(BlendMode::Add);
  sprite->SetAnchorPoint(Vector2{0.5f, 0.5f});

  // --- 3Dオブジェクト初期化 ---
  object3d2 = std::make_unique<Object3d>();
  object3d2->Initialize();
  object3d = std::make_unique<Object3d>();
  object3d->Initialize();

  ModelManager::GetInstance()->LoadModel("plane.obj");
  ModelManager::GetInstance()->LoadModel("axis.obj");
  object3d2->SetTranslate(Vector3{0.0f, 10.0f, 0.0f});
  object3d2->SetModel("axis.obj");
  object3d->SetModel("plane.obj");
  ModelManager::GetInstance()->CreateSphereModel("MySphere", 16);
  object3d->SetBlendMode(BlendMode::Add);

  Transform M = {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  emitter = std::make_unique<ParicleEmitter>("Test", M, 10, 5.0f, 0.0f);

  // --- マップチップ ---
  mapChipField_ = std::make_unique<MapChipField>();
  mapChipField_->LoadMapChipCsv("resources/stage2.csv");
  GenerateFieldObjects();
  if (player_) {
    player_->SetMapChipField(mapChipField_.get());
  }

  // --- チュートリアル状態初期化 ---
  currentPhase_ = TutorialPhase::kMovement;
  destroyedObstaclesCount_ = 0;
  isPhaseCleared_ = false;
  waitTimer_ = 0;
  isPhaseActive_ = false;

  // --- UIスプライト初期化 ---
  // ラムダで生成を簡略化
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
                                   {600.0f, 60.0f}, {640.0f, 600.0f});
  moveText_ = CreateSprite("resources/tutorial/move.png", {460.0f, 160.0f},
                           {640.0f, 100.0f}, false);
  clearedText_ = CreateSprite("resources/tutorial/cleared.png", {460.0f, 64.0f},
                              {640.0f, 100.0f}, false);
  obstacleExplanationSprite_ =
      CreateSprite("resources/tutorial/ObstacleDescription.png",
                   {640.0f, 500.0f}, {640.0f, 360.0f});
  destroyNumText_ = CreateSprite("resources/tutorial/destroyNum.png",
                                 {460.0f, 160.0f}, {640.0f, 100.0f}, false);
  driftExplanationSprite_ =
      CreateSprite("resources/tutorial/driftExplanation.png", {640.0f, 280.0f},
                   {640.0f, 450.0f});
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

  // 数字スプライト
  for (int i = 0; i < 4; i++) {
    std::string path = "resources/numbers/" + std::to_string(i) + ".png";
    TextureManager::GetInstance()->LoadTexture(path);
    auto sScore = std::make_unique<Sprite>();
    sScore->Initialize(path);
    sScore->SetPosition({600.0f, 220.0f});
    sScore->SetSize({64.0f, 64.0f});
    sScore->SetAnchorPoint({0.5f, 0.5f});
    numberSprites_.push_back(std::move(sScore));

    auto sTarget = std::make_unique<Sprite>();
    sTarget->Initialize(path);
    sTarget->SetPosition({700.0f, 220.0f});
    sTarget->SetSize({64.0f, 64.0f});
    sTarget->SetAnchorPoint({0.5f, 0.5f});
    targetNumberSprites_.push_back(std::move(sTarget));
  }

  scoreDisplay_ = std::make_unique<Bitmappedfont>();
  scoreDisplay_->Initialize(&numberSprites_, camera.get());
  targetDisplay_ = std::make_unique<Bitmappedfont>();
  targetDisplay_->Initialize(&targetNumberSprites_, camera.get());

  // --- フェード ---
  fade_ = std::make_unique<Fade>();
  fade_->Initialize(camera.get());
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
  camera->Update();

  // 共通の背景オブジェクト更新
  object3d->Update();
  object3d2->Update();

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
        fade_->SetCamera(camera.get());
        fade_->Start(Fade::Phase::kFadeIn);
        break;
      case CompleteOption::kGoToSelect:
        GetSceneManager()->ChangeScene("SelectScene");
        break;
      case CompleteOption::kBackToTitle:
        GetSceneManager()->ChangeScene("TitleScene");
        break;
      }
    }
    break;
  }

  UpdateDebugGUI();
}

// ====================================================================
// メイン更新処理 (Active / Pausedの振り分け)
// ====================================================================
void TutorialScene::UpdateMainState() {
  // 完了フェーズかどうか
  bool isCompletePhase = (currentPhase_ == TutorialPhase::kComplete);

  // ゲームロジックを動かす条件:
  // 「フェーズがアクティブ」かつ「完了画面ではない」
  bool shouldRunGameLogic = isPhaseActive_ && !isCompletePhase;

  // --- 1. 開始待ちの入力処理 ---
  if (!isPhaseActive_ && !isCompletePhase) {
    if (Input::GetInstance()->TriggerKeyDown(DIK_SPACE) ||
        Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {
      isPhaseActive_ = true;
    }
  }

  // --- 2. ゲームロジック or 停止中の更新 ---
  if (shouldRunGameLogic) {
    UpdateGameLogic();
  } else {
    // 停止中も描画用に行列更新が必要
    UpdatePausedState(isCompletePhase);
  }

  // --- 3. 共通更新 (UI, Particle) ---
  if (emitter)
    emitter->Update();

  // デバッグ用カメラ移動 (必要であれば)
  /* ... */

  UpdateUI();
}

// プレイ中のロジック (移動、当たり判定、削除)
void TutorialScene::UpdateGameLogic() {
  // プレイヤー
  if (player_)
    player_->Update(true);

  // 障害物・壁・ゴール
  UpdateAllObstacles();
  for (auto &wall : walls_)
    wall->Update();
  for (auto &goal : goals_)
    goal->Update();

  // 当たり判定
  CheckAllCollisions();

  // 死亡判定
  if (player_ && player_->IsDead()) {
    if (player_->IsDeathAnimationFinished()) {
      ResetTutorialState();
    }
    return;
  }

  // 進行チェック
  UpdateTutorialSteps();

  // 破壊された障害物の削除
  RemoveDeadObstacles();
}

// 停止中のロジック (行列計算のみ、Complete時はメニュー操作も)
void TutorialScene::UpdatePausedState(bool isCompletePhase) {
  // --- プレイヤーの更新 ---
  if (player_) {
    // 1. 内部状態の更新（移動はさせない）
    player_->Update(false);

    // 2. ★重要★ プレイヤーのモデルを強制的に更新
    // playerModel_ は Initialize で生成されているので、これを直接 Update します
    if (playerModel_) {
      playerModel_->Update();
    }
  }

  // 各障害物のUpdateを呼ぶ
  UpdateAllObstacles();

  for (auto &wall : walls_)
    wall->Update();
  for (auto &goal : goals_)
    goal->Update();

  // Complete画面なら、メニュー操作のために進行更新を呼ぶ
  if (isCompletePhase) {
    UpdateTutorialSteps();
  }
}

// 障害物一括更新
void TutorialScene::UpdateAllObstacles() {
  for (auto &obj : obstacleSlow_)
    obj->Update();
  for (auto &obj : obstacleNormal_)
    obj->Update();
  for (auto &obj : obstacleFast_)
    obj->Update();
  for (auto &obj : obstacleMax_)
    obj->Update();
}

// 障害物削除処理
void TutorialScene::RemoveDeadObstacles() {
  auto RemoveFromContainer = [&](auto &container) {
    size_t prevSize = container.size();
    container.erase(
        std::remove_if(container.begin(), container.end(),
                       [](const auto &obj) { return obj->IsScoreNone(); }),
        container.end());

    // Attackフェーズのみカウントを加算
    if (currentPhase_ == TutorialPhase::kAttack) {
      destroyedObstaclesCount_ += static_cast<int>(prevSize - container.size());
    }
  };

  RemoveFromContainer(obstacleSlow_);
  RemoveFromContainer(obstacleNormal_);
  RemoveFromContainer(obstacleFast_);
  RemoveFromContainer(obstacleMax_);
}

// UI更新
void TutorialScene::UpdateUI() {
  int displayNum = std::min(destroyedObstaclesCount_, 9);
  if (scoreDisplay_) {
    scoreDisplay_->SetNumber(displayNum);
    scoreDisplay_->Update();
  }
  int targetNum = std::min(kTargetDestroyCount_, 9);
  if (targetDisplay_) {
    targetDisplay_->SetNumber(targetNum);
    targetDisplay_->Update();
  }

  if (startGuideSprite_)
    startGuideSprite_->Update();
  if (moveText_)
    moveText_->Update();
  if (backTitleText_)
    backTitleText_->Update();
  if (clearedText_)
    clearedText_->Update();
  if (obstacleExplanationSprite_)
    obstacleExplanationSprite_->Update();
  if (destroyNumText_)
    destroyNumText_->Update();
  if (driftExplanationSprite_)
    driftExplanationSprite_->Update();
  if (driftText_)
    driftText_->Update();
  if (meterImpactText_)
    meterImpactText_->Update();
  if (retryText_)
    retryText_->Update();
  if (stageSelectText_)
    stageSelectText_->Update();
  if (tutorialClearedText_)
    tutorialClearedText_->Update();

  if (sprite)
    sprite->Update();
}

void TutorialScene::Draw() {
  object3d2->Draw();
  object3d->Draw();
  ParticleManager::GetInstance()->Draw();

  if (player_)
    player_->Draw();

  // 障害物描画
  DrawAllObstacles();

  for (auto &wall : walls_)
    wall->Draw();
  for (auto &goal : goals_)
    goal->Draw();

  // UI描画
  if (!isPhaseActive_ && currentPhase_ != TutorialPhase::kComplete &&
      !isPhaseCleared_) {

    if (currentPhase_ == TutorialPhase::kDrift) {
      // ドリフト開始前の説明
      if (driftExplanationSprite_)
        driftExplanationSprite_->Draw();
    } else if (currentPhase_ == TutorialPhase::kAttack) {
      // ★追加：障害物破壊（攻撃）開始前の説明
      if (obstacleExplanationSprite_)
        obstacleExplanationSprite_->Draw();
    } else {
      // それ以外のフェーズ（移動など）は通常のガイド
      if (startGuideSprite_)
        startGuideSprite_->Draw();
    }
  }

  if (currentPhase_ == TutorialPhase::kAttack && isPhaseActive_) {
    if (meterImpactText_)
      meterImpactText_->Draw();
  }

  if (isPhaseCleared_) {
    if (clearedText_)
      clearedText_->Draw();
  } else {
    switch (currentPhase_) {
    case TutorialPhase::kMovement:
      if (moveText_)
        moveText_->Draw();
      break;
    case TutorialPhase::kDrift:
      if (driftText_)
        driftText_->Draw();
      break;
    case TutorialPhase::kAttack:
      if (isPhaseActive_) {
        if (destroyNumText_)
          destroyNumText_->Draw();
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

  if (fade_)
    fade_->Draw();
  // if(sprite) sprite->Draw();
}

void TutorialScene::DrawAllObstacles() {
  for (auto &o : obstacleSlow_)
    o->Draw();
  for (auto &o : obstacleNormal_)
    o->Draw();
  for (auto &o : obstacleFast_)
    o->Draw();
  for (auto &o : obstacleMax_)
    o->Draw();
}

// ====================================================================
// 当たり判定
// ====================================================================
void TutorialScene::CheckAllCollisions() {
  if (!isPhaseActive_ || !player_)
    return;

  AABB aabbPlayer = player_->GetAABB();

  // ラムダで共通化
  auto CheckCollisionList = [&](auto &list) {
    for (auto &obj : list) {
      if (isCollision(aabbPlayer, obj->GetAABB())) {
        player_->OnCollision(obj.get());
        obj->OnCollision(player_.get());
      }
    }
  };

  CheckCollisionList(obstacleSlow_);
  CheckCollisionList(obstacleNormal_);
  CheckCollisionList(obstacleFast_);
  CheckCollisionList(obstacleMax_);

  // 壁
  for (auto &wall : walls_) {
    if (isCollision(aabbPlayer, wall->GetAABB())) {
      player_->OnCollision(wall.get());
    }
  }
  // ゴール
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

// ====================================================================
// チュートリアル進行
// ====================================================================
void TutorialScene::UpdateTutorialSteps() {
  // 1. クリア後の待機処理
  if (isPhaseCleared_) {
    waitTimer_--;
    if (waitTimer_ <= 0) {
      isPhaseCleared_ = false;
      // フェーズ遷移
      switch (currentPhase_) {
      case TutorialPhase::kMovement:
        currentPhase_ = TutorialPhase::kDrift;
        isPhaseActive_ = false; // 次のために一時停止
        break;
      case TutorialPhase::kDrift:
        currentPhase_ = TutorialPhase::kAttack;
        destroyedObstaclesCount_ = 0;
        isPhaseActive_ = false; // 次のために一時停止
        break;
      case TutorialPhase::kAttack:
        currentPhase_ = TutorialPhase::kComplete;
        isPhaseActive_ = false; // 完了画面は停止状態で処理
        break;
      default:
        break;
      }
    }
    return;
  }

  // 2. フェーズごとのクリア条件チェック
  if (player_) {
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
    case TutorialPhase::kComplete:
      // メニュー操作処理
      // カーソル移動
      if (Input::GetInstance()->TriggerKeyDown(DIK_W) ||
          Input::GetInstance()->TriggerKeyDown(DIK_UP)) {
        if (currentOption_ == CompleteOption::kRetry)
          currentOption_ = CompleteOption::kGoToSelect;
        else if (currentOption_ == CompleteOption::kBackToTitle)
          currentOption_ = CompleteOption::kRetry;
      }
      if (Input::GetInstance()->TriggerKeyDown(DIK_S) ||
          Input::GetInstance()->TriggerKeyDown(DIK_DOWN)) {
        if (currentOption_ == CompleteOption::kGoToSelect)
          currentOption_ = CompleteOption::kRetry;
        else if (currentOption_ == CompleteOption::kRetry)
          currentOption_ = CompleteOption::kBackToTitle;
      }

      // 色更新
      Vector4 colorSelected = {1.0f, 1.0f, 1.0f, 1.0f};
      Vector4 colorUnselected = {1.0f, 1.0f, 1.0f, alpha_};
      retryText_->SetColor(currentOption_ == CompleteOption::kRetry
                               ? colorSelected
                               : colorUnselected);
      stageSelectText_->SetColor(currentOption_ == CompleteOption::kGoToSelect
                                     ? colorSelected
                                     : colorUnselected);
      backTitleText_->SetColor(currentOption_ == CompleteOption::kBackToTitle
                                   ? colorSelected
                                   : colorUnselected);

      // 決定
      if (Input::GetInstance()->TriggerPadDown(0, XINPUT_GAMEPAD_A) ||
          Input::GetInstance()->TriggerKeyDown(DIK_SPACE)) {
        sceneState_ = SceneState::kFadeOut;
        fade_->Start(Fade::Phase::kFadeOut);
      }
      break;
    }
  }
}

// ====================================================================
// オブジェクト生成・リセット
// ====================================================================
void TutorialScene::ResetTutorialState() {
  player_.reset();
  playerModel_.reset();
  obstacleSlow_.clear();
  obstacleSlowModel_.clear();
  obstacleNormal_.clear();
  obstacleNormalModel_.clear();
  obstacleFast_.clear();
  obstacleFastModel_.clear();
  obstacleMax_.clear();
  obstacleMaxModel_.clear();
  walls_.clear();
  wallModels_.clear();
  goals_.clear();
  goalModels_.clear();

  isPhaseCleared_ = false;
  waitTimer_ = 0;
  destroyedObstaclesCount_ = 0;

  GenerateFieldObjects();
}

// TutorialScene.cpp

void TutorialScene::GenerateFieldObjects() {
  uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
  uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();
  worldTransformObjects.resize(numBlockVirtical);
  for (uint32_t i = 0; i < numBlockVirtical; ++i) {
    worldTransformObjects[i].resize(numBlockHorizontal);
  }

  // --- ヘルパー: モデルとオブジェクトを生成してコンテナに入れる ---
  auto CreateObstacle = [&](auto &list, auto &modelList, const char *modelName,
                            const Vector3 &pos, int type) {
    auto model = std::make_unique<Object3d>();
    model->SetModel(modelName);
    model->Initialize();

    using ObjType = typename std::remove_reference<
        decltype(*list.begin())>::type::element_type;
    auto obj = std::make_unique<ObjType>();

    // ★修正ポイント: ここで player_ が nullptr
    // だと落ちるため、呼び出し順序を制御する
    obj->Initialize(model.get(), camera.get(), pos, player_.get());

    modelList.push_back(std::move(model));
    list.push_back(std::move(obj));
  };

  // =================================================================
  // ★パス1: まずプレイヤーだけを先に探して生成する (優先度高)
  // =================================================================
  for (uint32_t i = 0; i < numBlockVirtical; ++i) {
    for (uint32_t j = 0; j < numBlockHorizontal; j++) {
      MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);

      if (type == MapChipType::kPlayer) {
        Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j, i);
        player_ = std::make_unique<Player>();
        playerModel_ = std::make_unique<Object3d>();
        playerModel_->SetTranslate(pos);
        playerModel_->SetModel("cube.obj");
        playerModel_->Initialize();
        player_->Initialize(playerModel_.get(), camera.get(), pos);
      }
    }
  }

  // =================================================================
  // ★パス2: プレイヤーが存在することが保証された状態で、他を生成する
  // =================================================================
  for (uint32_t i = 0; i < numBlockVirtical; ++i) {
    for (uint32_t j = 0; j < numBlockHorizontal; j++) {
      MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);
      Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j, i);

      switch (type) {
      case MapChipType::kBlank:
        break;

      case MapChipType::kPlayer:
        // パス1で生成済みなので無視
        break;

      case MapChipType::kObstacle: {
        // 念のためプレイヤーがいるかチェック
        if (player_) {
          uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
          if (subID == 0)
            CreateObstacle(obstacleSlow_, obstacleSlowModel_, "cube.obj", pos,
                           0);
          else if (subID == 1)
            CreateObstacle(obstacleNormal_, obstacleNormalModel_, "cube.obj",
                           pos, 1);
          else if (subID == 2)
            CreateObstacle(obstacleFast_, obstacleFastModel_, "cube.obj", pos,
                           2);
          else if (subID == 3)
            CreateObstacle(obstacleMax_, obstacleMaxModel_, "cube.obj", pos, 3);
        }
      } break;

      case MapChipType::kGoal: {
        auto gModel = std::make_unique<Object3d>();
        gModel->SetModel("cube.obj");
        gModel->Initialize();
        auto goal = std::make_unique<Goal>();
        goal->Initialize(gModel.get(), camera.get(), pos);
        goalModels_.push_back(std::move(gModel));
        goals_.push_back(std::move(goal));
      } break;

      case MapChipType::kWallStraight: {
        auto wModel = std::make_unique<Object3d>();
        wModel->SetModel("cube.obj");
        wModel->Initialize();
        auto wall = std::make_unique<CourseWall>();
        wall->Initialize(wModel.get(), camera.get(), pos);
        wall->SetScale({2.0f, 4.0f, 2.0f});
        wallModels_.push_back(std::move(wModel));
        walls_.push_back(std::move(wall));
      } break;

      case MapChipType::kWallTurn: {
        auto wModel = std::make_unique<Object3d>();
        wModel->SetModel("cube.obj");
        wModel->Initialize();
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
        wall->Initialize(wModel.get(), camera.get(), wallPos);
        wall->SetScale({2.0f, 4.0f, 2.0f});
        wallModels_.push_back(std::move(wModel));
        walls_.push_back(std::move(wall));
      } break;
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
    const char *phaseName = "";
    switch (currentPhase_) {
    case TutorialPhase::kMovement:
      phaseName = "Movement";
      break;
    case TutorialPhase::kDrift:
      phaseName = "Drift";
      break;
    case TutorialPhase::kAttack:
      phaseName = "Attack";
      break;
    case TutorialPhase::kComplete:
      phaseName = "Complete";
      break;
    }
    ImGui::Text("Phase: %s", phaseName);
    if (currentPhase_ == TutorialPhase::kComplete) {
      ImGui::Text("Option: %d", (int)currentOption_);
    }
  }

  // デバッグ機能（前のコードにあったもの）
  ImGui::Begin("Debug");
  if (object3d) {
    Vector3 pos = object3d->GetTranslate();
    Vector3 scale = object3d->GetScale();
    ImGui::SliderFloat3("Pos", &(pos.x), 0.1f, 1000.0f);
    ImGui::DragFloat3("scale", &(scale.x), 0.1f, 1000.0f);
    object3d->SetTranslate(pos);
    object3d->SetScale(scale);
  }
  ImGui::End();
  ImGui::End();
#endif
}