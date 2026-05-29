#pragma once
#include <vector>
#include <string>
#include "Vector3.h"

struct Vector4 {
    float x;
    float y;
    float z;
    float w;

    // Vector4 -> Vector3 への簡易変換（w を無視）
    Vector3 ToVector3() const {
        return Vector3{ x, y, z };
    }
};

struct Matrix4x4 {
   float m[4][4];
  
};
struct Matrix3x3{
   float m[3][3];
};

// ワールド行列で Vector3 を変換するユーティリティ関数
// 行優先 (row-major) を想定し、右項に同次座標 w=1 を使う実装
inline Vector3 Transform(const Vector3& v, const Matrix4x4& mat) {
    Vector3 out;
    out.x = mat.m[0][0] * v.x + mat.m[0][1] * v.y + mat.m[0][2] * v.z + mat.m[0][3];
    out.y = mat.m[1][0] * v.x + mat.m[1][1] * v.y + mat.m[1][2] * v.z + mat.m[1][3];
    out.z = mat.m[2][0] * v.x + mat.m[2][1] * v.y + mat.m[2][2] * v.z + mat.m[2][3];
    return out;
}


