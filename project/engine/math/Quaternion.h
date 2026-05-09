#pragma once
#include "Vector2.h"
struct Quaternion{
	float x=0;
	float y=0;
	float z=0;
	float w=1;
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