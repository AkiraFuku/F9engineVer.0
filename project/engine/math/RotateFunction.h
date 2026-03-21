#pragma once
#include "Vector4.h"
#include "Quanternion.h"


Quaternion Slerp(const Quaternion& q0,const Quaternion& q1,float t);


Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis,float angle);

/// <summary>
/// 
/// </summary>
/// <param name="vector"></param>
/// <param name="quaternion"></param>
/// <returns></returns>
/// /// ベクトルをクォータニオンで回転させる関数
Vector3 RotateVector(const Vector3& vector ,const Quaternion& quaternion);

Matrix4x4 MakeRotateMatrix(const Quaternion& q);

Matrix4x4 MakeRotateAxisAngle(const Vector3& axis,float angle);

Matrix4x4 DirectionToDirection(const Vector3& from, const Vector3& to);