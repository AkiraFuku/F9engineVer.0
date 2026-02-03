#pragma once
#include"MathFunction.h"
#include "Sprite.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include "ParicleEmitter.h"
#include "Audio.h"
#include "TextureManager.h"
#include "Scene.h"
#include <memory>
class GameScene :public Scene
{
public:
    void Initialize() override;
    void Finalize()override;
    void Update()override;
    void Draw()override;
private:
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Sprite> sprite;
    std::unique_ptr<Object3d> object3d2;
    std::unique_ptr<Object3d> object3d3;
    std::unique_ptr<Object3d> object3d;
    std::unique_ptr<ParicleEmitter> emitter;
  uint32_t handle_=0;

    Vector3 position = { 0.0f, 0.0f, 0.0f }; 
};

