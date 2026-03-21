#pragma once
#include "Vector2.h"
struct Quaternion{
	float x;
	float y;
	float z;
	float w;
};

Quaternion operator-(const Quaternion q);
Quaternion operator*(const Quaternion& q1,const Quaternion& q2);

Quaternion Multiply(const Quaternion& lhs,const Quaternion& rhs );
Quaternion identityQuaternion();
Quaternion Conjugate(const Quaternion& quaternion);
float Norm(const Quaternion& quaternion);
Quaternion Normalize(const Quaternion& quaternion);
Quaternion Inverse(const Quaternion& quaternion);

//void  QuaternionScreenPrintf(Vector2 pos,const Quaternion& quaternion, const char* label);