#include "Quanternion.h"
#include "MathFunction.h"
#include <cmath>
Quaternion operator-(const Quaternion q)
{
	return {-q.x, -q.y, -q.z, -q.w};
	
}
Quaternion operator*(const Quaternion& q1, const Quaternion& q2)
{
	return Multiply(q1,q2);
}
Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs)
{
	Quaternion result;
	result.w = (lhs.w * rhs.w) - (lhs.x * rhs.x) - (lhs.y * rhs.y) - (lhs.z * rhs.z);
	result.x = (lhs.y * rhs.z) - (lhs.z * rhs.y) + (lhs.x * rhs.w) + (lhs.w * rhs.x);
	result.y = (lhs.z * rhs.x) - (lhs.x * rhs.z) + (lhs.y * rhs.w) + (lhs.w * rhs.y);
	result.z = (lhs.x * rhs.y) - (lhs.y * rhs.x) + (lhs.z * rhs.w) + (lhs.w * rhs.z);
	return result;
}

Quaternion identityQuaternion()
{
	Quaternion result;

	result.w = 1.0f;
	result.x = 0.0f;
	result.y = 0.0f;
	result.z = 0.0f;

	return result;
}

Quaternion Conjugate(const Quaternion& quaternion)
{

	Quaternion result;
	result.w = quaternion.w;
	result.x = -quaternion.x;
	result.y = -quaternion.y;
	result.z = -quaternion.z;

	return result;
}

float Norm(const Quaternion& quaternion)
{
	return std::sqrtf(std::powf(quaternion.w, 2.0f) + std::powf(quaternion.x, 2.0f) + std::powf(quaternion.y, 2.0f) + std::powf(quaternion.z, 2.0f));
}

Quaternion Normalize(const Quaternion& quaternion)
{
	float norm = Norm(quaternion);
	Quaternion result;
	if (norm != 0.0f) {
        result.w = quaternion.w / norm;
        result.x = quaternion.x / norm;
        result.y = quaternion.y / norm;
        result.z = quaternion.z / norm;
    } else {
        result = {0.0f, 0.0f, 0.0f, 0.0f}; 
    }
	return result;
}

Quaternion Inverse(const Quaternion& quaternion)
{
	float norm = Norm(quaternion);
	Quaternion conjugate = Conjugate(quaternion);
	Quaternion result;
	result.w = conjugate.w / std::powf(norm, 2.0f);
	result.x = conjugate.x / std::powf(norm, 2.0f);
	result.y = conjugate.y / std::powf(norm, 2.0f);
	result.z = conjugate.z / std::powf(norm, 2.0f);
	return result;
}

//void QuaternionScreenPrintf(Vector2 pos, const Quaternion& quaternion, const char* label)
//{
//	Novice::ScreenPrintf(static_cast<int>(pos.x), static_cast<int>(pos.y), "%.02f", quaternion.x);
//	Novice::ScreenPrintf(static_cast<int>(pos.x + kColumnWidth), static_cast<int>(pos.y), "%.02f ", quaternion.y);
//	Novice::ScreenPrintf(static_cast<int>(pos.x + kColumnWidth * 2), static_cast<int>(pos.y), "%.02f ", quaternion.z);
//	Novice::ScreenPrintf(static_cast<int>(pos.x + kColumnWidth * 3),static_cast<int>( pos.y), "%.02f ", quaternion.w);
//	Novice::ScreenPrintf(static_cast<int>(pos.x + kColumnWidth * 4),static_cast<int>( pos.y), ":%s", label);
//}
