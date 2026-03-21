#pragma once
#pragma once

#include"Object3D.h"
#include "Model.h"
#include "Camera.h"

class Terrain
{
public:

    void Initialize();

    void Finalize();

    void Update();

    void Draw();

public: // 外部入出力





private:

    std::unique_ptr<Object3d> object_;

    Vector3 scale_ = { 1.0f,1.0f,1.0f };
    Vector3 rotate_ = { 0.0f,0.0f,0.0f };
    Vector3 translate_ = { 0.0f,0.0f,0.0f };
};


