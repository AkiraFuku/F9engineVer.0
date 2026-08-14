#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include "Sprite.h"
#include"Object3D.h"

#include "Player.h"
#include "Scene.h"
#include <memory>
#include "SkyBox.h"
#include "CameraController.h"
#include "Vector2.h"
#include "Projectile.h"
#include "Animation.h"
#include "Enemy.h"

class ParticleEmitter;
class RailPath;
class GoalObject;
class Phase;
class PlayerHPUI;
class ScoreUI;
class MiniBoss;

class GameScene :public Scene
{
public:
    void Initialize()override;
    void Finalize()override;
    void Update()override;
    void Draw()override;

    /// <summary>
    /// フェーズの切り替え条件を満たしているかチェックする
    /// </summary>
    void CheckPhaseTransition();
    /// <summary>
    /// フェーズを変更する
    /// </summary>
    /// <param name="nextPhase"></param>
    void ChangePhase(std::unique_ptr<Phase> nextPhase);


    //プレイヤーが落下下かの判定
    void CheckPlayerFall();


    Player* GetPlayer() {
        return player.get();
    }
    CameraController* GetCamera() {
        return cameraController.get();
    }
    RailPath* GetStageRaill() {
        return stageRail.get();
    }
    const std::vector<std::unique_ptr<Enemy>>& GetEnemies() {
        return enemies_;
    }
    const std::vector<std::unique_ptr<Projectile>>& GetProjectile() {
        return projectiles_;
    }
    const std::vector<Triangle>& GetTriangle() {
        return triangles_;
    }
    GoalObject* GetGoal() {
        return goal_.get();
    }

    GameScene();
    ~GameScene() override;

    void AddEnemy(Vector2 pos,Enemy::EnemyType enemyType =Enemy::EnemyType::Normal);
    void AddProjectile(const Projectile::ProjectileSpawnParam& param, Projectile::ProjectileOwner owner);
    void AddTriangles(std::vector<Triangle> triangles);

private:
    /// <summary>
    /// クリアフラグが立ったら遷移
    /// </summary>
    void CheckClear();
    std::unique_ptr<Object3d> object3d;
    std::unique_ptr<Phase> currentPhase_;

    std::unique_ptr<Animation> animation;
    std::unique_ptr<Player> player;
    std::unique_ptr<RailPath> stageRail;
    std::unique_ptr<RailPath> cameraRail;

    std::unique_ptr<MiniBoss> miniBoss_;

    std::unique_ptr<CameraController> cameraController;
    std::vector<std::unique_ptr<Enemy>> enemies_;
    std::unique_ptr<SkyBox> skyBox;
    std::unique_ptr<CameraController> debugCameraC;

    bool isDebugCamera_ = false;
    std::unique_ptr<ParticleEmitter>emitter_;

    std::unique_ptr<GoalObject> goal_;

    Vector3 position_ = { 2.0f,0.0f,0.0f };
    Quaternion rotation_ = { 0.0f,0.0f,0.0f,1.0f };

    bool isCleared_ = false;
    bool isDefeated_ = false;
    uint32_t handle_ = 0;
    // 投射物リスト
    std::vector<std::unique_ptr<Projectile>> projectiles_;
    //シーン内三角形リスト
    std::vector<Triangle>triangles_;

    // テスト用レイキャスト衝突判定メンバ変数
    std::unique_ptr<Object3d> boxObject_;
    bool isBoxHit_ = false;
    Vector3 boxPoint_ = { 0.0f,0.0f,0.0f };
    Vector3 boxHitPoint_ = {};
    float boxHitDistance_ = 0.0f;
    Triangle hitTriangle_ = {};
    Ray debugRay_ = {};

    //テスト用地面
    std::unique_ptr<Object3d> TestGround_;
    //生存限界
    float fallLimit_ = -8.5f;

    //UI
    std::unique_ptr<PlayerHPUI> playerHPUI_;
    std::unique_ptr<ScoreUI> scoreUI_;

    //ヒットストップ用変数
    bool isHitStop_ = false;


    // GameScene.h (privateメンバ変数に追加)
    float stopTimer_ = 0; // 残りヒットストップフレーム数

    void UpdateHitStop() ;

public:
    // ヒットストップを開始する関数
    void TriggerHitStop(float frames = 0.1f) {
        stopTimer_ = frames;
    }
    bool IsHitStopActive() const {
        return stopTimer_  > 0;
    }
};

