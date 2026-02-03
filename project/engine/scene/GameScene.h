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

class Score;
class Bitmappedfont;
class Goal;

class CourseWall;

#include"Fade.h"

class GameScene : public Scene {
public:
    void Initialize() override;
    void Finalize() override;
    void Update() override;
    void Draw() override;
    GameScene();
    ~GameScene() override;

    // 全ての当たり判定を行う
    void CheckAllCollisions();
    // 当たり判定
    bool isCollision(const AABB& aabb1, const AABB& aabb2);

    // マップチップの生成
    void GenerateFieldObjects();

    // 開始フラグを取得
    bool IsStarted() const { return isStarted_; }
    // クリアフラグをセット
    void SetCleared(bool isCleared) { isCleared_ = isCleared; }
    // ゲームオーバーフラグをセット
    void SetGameOver(bool isGameOver) { isGameOver_ = isGameOver; }

    // スコアをセット
    void SetScore(int score,int num,int count);


private:
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Sprite> sprite;
    std::unique_ptr<Object3d> object3d2;
    std::unique_ptr<Object3d> object3d;
    std::unique_ptr<ParicleEmitter> emitter;
    Audio::SoundHandle handle_;

    // 背景のモデル
    std::unique_ptr<Object3d> backgroundModel_;

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


    // ビットマップフォント
    std::unique_ptr<Bitmappedfont> bitmappedFont_;
    std::vector<std::unique_ptr<Sprite>> bitmappedFontSprites_;

    // ゴール
    std::vector<std::unique_ptr<Goal>> goals_;
    std::vector<std::unique_ptr<Object3d>> goalModels_;

    // スタート前のカウントダウン
    int32_t countdownTimer_ = 180;
    // ゲーム開始フラグ
    bool isStarted_ = false;
    // クリアフラグ
    bool isCleared_ = false;
    // ゲームオーバーフラグ
    bool isGameOver_ = false;

    // テキスト
    std::unique_ptr<Sprite> clearText_;
    std::unique_ptr<Sprite> gameOverText_;
    std::unique_ptr<Sprite> scoreText_;
    std::unique_ptr<Sprite> pressSpaceText_;

    // スコア
    std::vector < std::unique_ptr<Bitmappedfont> > scoreBitmappedFonts_;

    // マップチップフィールド
    std::unique_ptr<MapChipField> mapChipField_;
    // ブロック用のワールドトランスフォーム
    std::vector<std::vector<Transform*>> worldTransformObjects;

// メンバ
    std::unique_ptr<Fade> fade_;
    bool requestSceneChange_ = false;


};
