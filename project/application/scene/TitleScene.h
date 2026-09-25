#pragma once
#include"MathFunction.h"
#include "Sprite.h"
#include"Object3D.h"
#include "Model.h"
#include "Camera.h"
#include "ParticleEmitter.h"
#include "Audio.h"
#include "TextureManager.h"

#include "Scene.h"
#include <memory>
#include "Audio.h"

#include "SkyBox.h"

#include "Animation.h"

class Phase;



class TitleScene :public Scene
{
public:
    TitleScene();
    ~TitleScene();

    void Initialize()override;
    void Finalize()override;
    void Update()override;
    void Draw()override;


private:
    std::unique_ptr<Camera> camera;

    std::unique_ptr<Sprite> sprite;
    std::unique_ptr<Object3d> object3d;
    std::unique_ptr<Animation> animation;

    bool isDebugCamera_ = false;
    std::unique_ptr<SkyBox> skyBox;

     Audio::SoundHandle  handle_ = 0;
    std::unique_ptr<class PbdCloth> cloth_;
    bool clothWindEnabled_ = true;
    /// <summary>
    /// 布の　風の強さ（揺れの振幅）
    /// </summary>
    float clothWindStrength_ = 3.5f;
    float clothTimer_ = 0.0f;
};

