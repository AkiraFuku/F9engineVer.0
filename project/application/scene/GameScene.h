#pragma once
#include"MathFunction.h"
#include "Sprite.h"
#include"Object3D.h"
#include "Player.h"
#include "Scene.h"
#include <memory>
#include "CameraController.h"

class RailPath;

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


private:
    std::unique_ptr<Sprite> sprite;
    std::unique_ptr<Object3d> object3d;
    std::unique_ptr<Animation> animation;
    std::unique_ptr<Player> player;
    std::unique_ptr<RailPath> stageRail;

    std::unique_ptr<CameraController> cameraController; 

    bool isDebugCamera_ = false;

    Vector3 position_ = { 2.0f,0.0f,0.0f };
    Quaternion rotation_ ={ 0.0f,0.0f,0.0f,1.0f };
    /*Vector3 point1_ = { 0.0f,0.0f,0.0f };
    Vector3 point2_ = { 0.0f,0.0f,50.0f };*/

     uint32_t handle_=0;
};

