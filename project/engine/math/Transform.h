#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "MathFunction.h"
struct EulerTransform{
    Vector3 scale;
    Vector3 rotate;
    Vector3 translate;

    //Matrix4x4 GetAffineMatrix() const;
};
struct QuaternionTransform{
    Vector3 scale;
   Quaternion rotate;
    Vector3 translate;
//    Matrix4x4 GetAffineMatrix() const;
};
// UV変換用の構造体
struct UVTransform
{
    Vector2 scale = {1.0f, 1.0f};
    float rotate = 0.0f;
    Vector2 offset = {0.0f, 0.0f};

    //Matrix4x4 GetUVTransformMatrix() const;
};