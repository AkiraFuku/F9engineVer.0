#include "RotateFunction.h"
#include "MathFunction.h"
#include "DrawFunction.h"
#include <numbers>
#include <cmath>


Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle)
{
	Quaternion result;
	Vector3 a = Normalize(axis);

	
	float halfAngle = angle * 0.5f;

	// 2. sin, cosを計算
	float s = std::sinf(halfAngle);
	float c = std::cosf(halfAngle);

	

	result.x = s*a.x;
	result.y = s*a.y;
	result.z = s*a.z;


	result.w = c;

	return result;
}

Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion)
{
	Quaternion p;
	p.x=vector.x;
	p.y=vector.y;
	p.z=vector.z;
	p.w=0.0f;

	Quaternion qInverse = Conjugate(quaternion);

   
    Quaternion temp = Multiply(quaternion, p);
    
   
    Quaternion resultQ = Multiply(temp, qInverse);

    
    Vector3 resultV;
    resultV.x = resultQ.x;
    resultV.y = resultQ.y;
    resultV.z = resultQ.z;

    return resultV;

}

Matrix4x4 MakeRotateMatrix(const Quaternion& q)
{
	Matrix4x4 result;

	result.m[0][0] =  powf(q.w,2) + powf(q.x,2) - powf(q.y,2) - powf(q.z,2) ;
	result.m[0][1] =  2 * (q.x * q.y + q.w * q.z) ;
	result.m[0][2] =  2 * (q.x * q.z - q.w * q.y) ;
	result.m[0][3] = 0;

	result.m[1][0] =  2 * (q.x * q.y - q.w * q.z);
	result.m[1][1] =  powf(q.w,2) - powf(q.x,2) + powf(q.y,2) - powf(q.z,2) ;
	result.m[1][2] =  2 * (q.y * q.z + q.w * q.x) ;
	result.m[1][3] = 0;

	result.m[2][0] =  2 * (q.x * q.z + q.w * q.y) ;
	result.m[2][1] =  2 * (q.y * q.z - q.w * q.x) ;
	result.m[2][2] =  powf(q.w,2) - powf(q.x,2) - powf(q.y,2) + powf(q.z,2) ;
	result.m[2][3] = 0;

	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = 0;
	result.m[3][3] = 1;

	return result;
}

Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle)
{
	Vector3 a = Normalize(axis);
	float x = a.x, y = a.y, z = a.z;

	float c = std::cos(angle);
	float s = std::sin(angle);
	float t = 1.0f - c;

	Matrix4x4 m{};

	m.m[0][0] = t * x * x + c;
	m.m[0][1] = t * x * y - s * z;
	m.m[0][2] = t * x * z + s * y;
	m.m[0][3] = 0;

	m.m[1][0] = t * x * y + s * z;
	m.m[1][1] = t * y * y + c;
	m.m[1][2] = t * y * z - s * x;
	m.m[1][3] = 0;

	m.m[2][0] = t * x * z - s * y;
	m.m[2][1] = t * y * z + s * x;
	m.m[2][2] = t * z * z + c;
	m.m[2][3] = 0;

	m.m[3][0] = 0;
	m.m[3][1] = 0;
	m.m[3][2] = 0;
	m.m[3][3] = 1;

	return m;
}

Matrix4x4 DirectionToDirection(const Vector3& from, const Vector3& to)
{
	// 安全のためまず正規化
	Vector3 f = Normalize(from);
	Vector3 t = Normalize(to);

	float c = Dot(f, t);
	float s = Length(Cross(f, t));

	// 完全に同方向 (0度)
	if (c > 0.9999f) {
		return Makeidetity4x4();
	}

	// 完全に反対方向 (180度)
	if (c < -0.9999f) {
		// from に直交する安定した軸を生成
		Vector3 ortho =
			(std::abs(f.x) > std::abs(f.z)) ?
			Vector3{ -f.y, f.x, 0.0f } :
			Vector3{ 0.0f, -f.z, f.y };

		ortho = Normalize(ortho);
		return MakeRotateAxisAngle(ortho, std::numbers::pi_v<float>);
	}

	// 一般ケース: 軸 = f × t
	Vector3 axis = Normalize(Cross(f, t));

	// 角度 = atan2(|f×t|, dot(f,t))
	float angle = std::atan2(s, c);

	return MakeRotateAxisAngle(axis, angle);
}

Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t)
{

	float dot = q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
	Quaternion targetQ0 = q0;
	if (dot<0.0f)
	{
		targetQ0 = -targetQ0;
		dot=-dot;
	}
	float theta=std::acosf(dot);
	

	
	float scale0=std::sinf((1.0f-t)*theta)/std::sinf(theta);
	float scale1=std::sinf(t*theta)/std::sinf(theta);


	Quaternion result{};
    result.x = scale0 * targetQ0.x + scale1 * q1.x;
    result.y = scale0 * targetQ0.y + scale1 * q1.y;
    result.z = scale0 * targetQ0.z + scale1 * q1.z;
    result.w = scale0 * targetQ0.w + scale1 * q1.w;

	return result;
}
