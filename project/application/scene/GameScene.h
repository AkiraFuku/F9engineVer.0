#pragma once
#include"MathFunction.h"
#include "Sprite.h"
#include"Object3D.h"
#include "Player.h"
#include "Scene.h"
#include <memory>
#include "SkyBox.h"
#include "CameraController.h"
#include "Vector2.h"
#include "Projectile.h"

class ParticleEmitter;
class RailPath;
class Enemy;
class GoalObject;
class Phase;

#include "Animation.h"

class GameScene :public Scene
{
public:
    void Initialize()override;
    void Finalize()override;
    void Update()override;
    void Draw()override;

    Player* GetPlayer(){return player.get();}
    CameraController* GetCamera(){return cameraController.get();}
    RailPath* GetStageRaill(){return stageRail.get();}
   const std::vector<std::unique_ptr<Enemy>>& GetEnemies(){return enemies_;}
   const std::vector<std::unique_ptr<Projectile>>& GetProjectile(){return projectiles_;}
    GoalObject* GetGoal(){return goal_.get(); }

    GameScene() ;
    ~GameScene() override;

    void AddEnemy(Vector2 pos);
void AddProjectile(const Projectile::ProjectileSpawnParam& param,Projectile::ProjectileOwner owner);

private:
    /// <summary>
    /// クリアフラグが立ったら遷移
    /// </summary>
    void CheckClear();
    std::unique_ptr<Object3d> object3d;
    std::unique_ptr<Phase> currentPhase_;
    std::unique_ptr<Phase> nextPhase_;

    std::unique_ptr<Animation> animation;
    std::unique_ptr<Player> player;
    std::unique_ptr<RailPath> stageRail;
    std::unique_ptr<RailPath> cameraRail;

    std::unique_ptr<CameraController> cameraController; 
    std::vector<std::unique_ptr<Enemy>> enemies_;
    std::unique_ptr<SkyBox> skyBox;
    std::unique_ptr<CameraController> debugCameraC; 

    bool isDebugCamera_ = false;
    std::unique_ptr<ParticleEmitter>emitter_;

    std::unique_ptr<GoalObject> goal_;

    Vector3 position_ = { 2.0f,0.0f,0.0f };
    Quaternion rotation_ ={ 0.0f,0.0f,0.0f,1.0f };
 
    bool isCleared_=false;
     uint32_t handle_=0;
     std::vector<std::unique_ptr<Projectile>> projectiles_;
};

