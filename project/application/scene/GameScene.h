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

class RailPath;
class Enemy;
#include "Animation.h"

class GameScene :public Scene
{
public:
    void Initialize()override;
    void Finalize()override;
    void Update()override;
    void Draw()override;
    GameScene() ;
    ~GameScene() override;

    void AddEnemy(Vector2 pos);
void AddProjectile(const Projectile::ProjectileSpawnParam& param);

private:
    std::unique_ptr<Sprite> sprite;
    std::unique_ptr<Object3d> object3d;
    std::unique_ptr<Animation> animation;
    std::unique_ptr<Player> player;
    std::unique_ptr<RailPath> stageRail;
    std::unique_ptr<RailPath> cameraRail;

    std::unique_ptr<CameraController> cameraController; 
    std::vector<std::unique_ptr<Enemy>> enemies_;
   // std::unique_ptr<Enemy> enemy; 
    std::unique_ptr<SkyBox> skyBox;
    std::unique_ptr<CameraController> debugCameraC; 

    bool isDebugCamera_ = false;

    Vector3 position_ = { 2.0f,0.0f,0.0f };
    Quaternion rotation_ ={ 0.0f,0.0f,0.0f,1.0f };
    /*Vector3 point1_ = { 0.0f,0.0f,0.0f };
    Vector3 point2_ = { 0.0f,0.0f,50.0f };*/

     uint32_t handle_=0;
     std::vector<std::unique_ptr<Projectile>> projectiles_;
};

