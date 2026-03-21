#include "MathFunction.h"
#include "RotateFunction.h"
//
// 
// 
// 
// 
Matrix4x4 MakeBillboardMatrix(const Vector3& scale, const Vector3& rotate, Matrix4x4& billboardMatrix, const Vector3& translate)
{
    // 1. 各成分の行列を作成
    Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
    Matrix4x4 rotateMatrix = MakeRotateZMatrix(rotate.z); // Z軸回転のみ使用
    Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

    // 2. 行列を順番に合成 (Scale * RotateZ * Billboard * Translate)

    // Scale * RotateZ
    Matrix4x4 matScaleRot = Multiply(scaleMatrix, rotateMatrix);

    // (Scale * RotateZ) * Billboard
    Matrix4x4 matSRB = Multiply(matScaleRot, billboardMatrix);

    // 全体 * Translate
    Matrix4x4 result = Multiply(matSRB, translateMatrix);

    return result;
}
Vector3 Cross(const Vector3& v1, const Vector3& v2)
{
	
    return {
    v1.y * v2.z - v1.z * v2.y,
    v1.z * v2.x - v1.x * v2.z,
    v1.x * v2.y - v1.y * v2.x
    };
}
Vector3 Division(const Vector3& v1, const Vector3& v2)
{
    return Vector3(
			v1.x / v2.x,
			v1.y / v2.y,
			v1.z / v2.z
		
		);
}
Vector3 operator+(const Vector3& v1, const Vector3& v2)
{


    Vector3 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    return result;
}

Vector3 operator+=(Vector3& v1, const Vector3& v2)
{

    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    return v1;
}

Vector3 operator-(const Vector3& v1, const Vector3& v2)
{

    Vector3 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    return result;
}

Vector3 operator-=(Vector3& v1, const Vector3& v2)
{

    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
    return v1;
}

Vector3 operator*(float scalar, const Vector3& v)
{

    Vector3 result;
    result.x = v.x * scalar;
    result.y = v.y * scalar;
    result.z = v.z * scalar;
    return result;
}

Vector3 operator*(const Vector3& v, float scalar)
{
    return scalar * v;

}
Vector3 operator/(const Vector3& v, float scalar)
{

    Vector3 result;
    result.x = v.x / scalar;
    result.y = v.y / scalar;
    result.z = v.z / scalar;
    return result;
}

Vector3 operator/=(Vector3& v, float scalar)
{
    v.x /= scalar;
    v.y /= scalar;
    v.z /= scalar;
    return v;
}
	

	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farCrip){
		float cot = 1.0f / std::tan(fovY / 2.0f);
		return Matrix4x4(
			{
				{{cot / aspectRatio,0.0f,0.0f,0.0f},
				{0.0f,cot,0.0f,0.0f},
				{0.0f,0.0f,farCrip  / (farCrip- nearClip),1.0f},
				{0.0f,0.0f,( -nearClip*farCrip) / (farCrip- nearClip), 0.0f}}
			
			}
		
		);
	}

	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farCrip){
		return Matrix4x4(
			{
				{2.0f / (right - left), 0.0f, 0.0f, 0.0f},
				{0.0f, 2.0f / (top - bottom), 0.0f, 0.0f},
				{0.0f, 0.0f, 1.0f / (farCrip - nearClip), 0.0f},
				{(right + left) / (left-right), (top + bottom) / ( bottom-top),  nearClip / ( nearClip-farCrip), 1.0f}
			
			}
		
		);
	}

	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth){
		return Matrix4x4(
			{
				{
					{width / 2.0f, 0.0f, 0.0f, 0.0f},
					{0.0f, -height / 2.0f, 0.0f, 0.0f},
					{0.0f, 0.0f, maxDepth - minDepth, 0.0f},
					{left + width / 2.0f, top + height / 2.0f, minDepth, 1.0f}
				}

			}

		);
	}

	Matrix4x4 MakeAfineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& traslate)
	{
		Matrix4x4 scaleMatrix=MakeScaleMatrix(scale);
		Matrix4x4 rotateMatrix=Multiply(MakeRotateXMatrix( rotate.x),Multiply(MakeRotateYMatrix( rotate.y),MakeRotateZMatrix( rotate.z)));
		Matrix4x4 traslateMatrix=MakeTranslateMatrix(traslate);

	    Matrix4x4 result=Multiply(Multiply(scaleMatrix,rotateMatrix),traslateMatrix);
		return result ;
	}

    Matrix4x4 MakeAfineMatrix(const Vector3& scale, const Quaternion& rotate, const Vector3& traslate)
    {
       Matrix4x4 scaleMatrix=MakeScaleMatrix(scale);
       Matrix4x4 rotateMatrix = MakeRotateMatrix(rotate);
		Matrix4x4 traslateMatrix=MakeTranslateMatrix(traslate);

	    Matrix4x4 result=Multiply(Multiply(scaleMatrix,rotateMatrix),traslateMatrix);
		return result ;
    }

  


	/// <summary>
/// 
/// </summary>
/// <param name="traslate"></param>
/// <returns></returns>
Matrix4x4 MakeTranslateMatrix(const Vector3& traslate){
	return Matrix4x4(
		{
			{
				{1.0f,0.0f,0.0f,0.0f},
				{0.0f,1.0f,0.0f,0.0f},
				{0.0f,0.0f,1.0f,0.0f},
				{traslate.x,traslate.y,traslate.z,1.0f}
			}
		}
	);
}
/// <summary>
/// 
/// </summary>
/// <param name="scale"></param>
/// <returns></returns>
Matrix4x4 MakeScaleMatrix(const Vector3& scale){
	return Matrix4x4(
		{
			{
				{scale.x,0.0f,0.0f,0.0f},
				{0.0f,scale.y,0.0f,0.0f},
				{0.0f,0.0f,scale.z,0.0f},
				{0.0f,0.0f,0.0f,1.0f}
			}
		}
	);
}
/// <summary>
/// 
/// </summary>
/// <param name="vector"></param>
/// <param name="matrix"></param>
/// <returns></returns>
Vector3 vector3Transform(const Vector3& vector,const Matrix4x4& matrix ){
	Vector3 result ;
	result.x=vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] +vector.z*matrix.m[2][0]+ 1.0f * matrix.m[3][0];
	result.y=vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] +vector.z*matrix.m[2][1]+ 1.0f * matrix.m[3][1];
	result.z=vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] +vector.z*matrix.m[2][2]+ 1.0f * matrix.m[3][2];
	float  w=vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] +vector.z*matrix.m[2][3]+ 1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}
/// <summary>
/// 
/// </summary>
/// <param name="radian"></param>
/// <returns></returns>
Matrix4x4 MakeRotateXMatrix(float radian)
{
	return Matrix4x4(
	{
		{1.0f,0.0f,0.0f,0.0f,},
		{0.0f,std::cos(radian),std::sin(radian),0.0f},	
		{0.0f,-std::sin(radian),std::cos(radian),0.0f},
		{0.0f,0.0f,0.0f,1.0f}
	}
	);
}
Matrix4x4 MakeRotateYMatrix(float radian)
{
	return Matrix4x4(
		{
		{std::cos(radian),0.0f,-std::sin(radian),0.0f,},
		{0.0f,1.0f,0.0f,0.0f},	
		{std::sin(radian),0.0f,std::cos(radian),0.0f},
		{0.0f,0.0f,0.0f,1.0f}
	}
	);
}
Matrix4x4 MakeRotateZMatrix(float radian)
{
	return Matrix4x4(
	{
		
		{std::cos(radian),std::sin(radian),0.0f,0.0f},	
		{-std::sin(radian),std::cos(radian),0.0f,0.0f},
		{0.0f,0.0f,1.0f,0.0f,},
		{0.0f,0.0f,0.0f,1.0f}
	}
	);
}
//
	Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2){
		Matrix4x4 result={};
		for (int i = 0; i < 4; i++){
			for(int j = 0; j < 4; j++){
				result.m[i][j]=m1.m[i][j]+m2.m[i][j];
			}
		}
		return result;
	}
	Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2){
		Matrix4x4 result={};
		for (int i = 0; i < 4; i++){
			for(int j = 0; j < 4; j++){
				result.m[i][j]=m1.m[i][j]-m2.m[i][j];
			}
		}
		return result;
	}
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2){
		Matrix4x4 result={};
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					result.m[i][j] += m1.m[i][k] * m2.m[k][j];
				}
			}
		}
		
		return result;
	}
	Matrix4x4 Inverse(const Matrix4x4& m){
			float det =
				(m.m[0][0]*m.m[1][1]*m.m[2][2]*m.m[3][3])+(m.m[0][0]*m.m[1][2]*m.m[2][3]*m.m[3][1])+(m.m[0][0]*m.m[1][3]*m.m[2][1]*m.m[3][2])
			-	(m.m[0][0]*m.m[1][3]*m.m[2][2]*m.m[3][1])-(m.m[0][0]*m.m[1][2]*m.m[2][1]*m.m[3][3])-(m.m[0][0]*m.m[1][1]*m.m[2][3]*m.m[3][2])
			-	(m.m[0][1]*m.m[1][0]*m.m[2][2]*m.m[3][3])-(m.m[0][2]*m.m[1][0]*m.m[2][3]*m.m[3][1])-(m.m[0][3]*m.m[1][0]*m.m[2][1]*m.m[3][2])
			+	(m.m[0][3]*m.m[1][0]*m.m[2][2]*m.m[3][1])+(m.m[0][2]*m.m[1][0]*m.m[2][1]*m.m[3][3])+(m.m[0][1]*m.m[1][0]*m.m[2][3]*m.m[3][2])
			+	(m.m[0][1]*m.m[1][2]*m.m[2][0]*m.m[3][3])+(m.m[0][2]*m.m[1][3]*m.m[2][0]*m.m[3][1])+(m.m[0][3]*m.m[1][1]*m.m[2][0]*m.m[3][2])
			-	(m.m[0][3]*m.m[1][2]*m.m[2][0]*m.m[3][1])-(m.m[0][2]*m.m[1][1]*m.m[2][0]*m.m[3][3])-(m.m[0][1]*m.m[1][3]*m.m[2][0]*m.m[3][2])
			-	(m.m[0][1]*m.m[1][2]*m.m[2][3]*m.m[3][0])-(m.m[0][2]*m.m[1][3]*m.m[2][1]*m.m[3][0])-(m.m[0][3]*m.m[1][1]*m.m[2][2]*m.m[3][0])
			+	(m.m[0][3]*m.m[1][2]*m.m[2][1]*m.m[3][0])+(m.m[0][2]*m.m[1][1]*m.m[2][3]*m.m[3][0])+(m.m[0][1]*m.m[1][3]*m.m[2][2]*m.m[3][0])
			;
		Matrix4x4 result=
		{
			{
			//0
				{///0
					(1 / det)*(
						 (m.m[1][1]*(m.m[2][2]*m.m[3][3]-m.m[2][3]*m.m[3][2]))
						+(m.m[1][2]*(m.m[2][3]*m.m[3][1]-m.m[2][1]*m.m[3][3]))
						+(m.m[1][3]*(m.m[2][1]*m.m[3][2]-m.m[2][2]*m.m[3][1]))
						),
				////

				 ///1
					-(1 / det)*(
						 (m.m[2][1]*(m.m[3][2]*m.m[0][3]-m.m[3][3]*m.m[0][2]))
						+(m.m[2][2]*(m.m[3][3]*m.m[0][1]-m.m[3][1]*m.m[0][3]))
						+(m.m[2][3]*(m.m[3][1]*m.m[0][2]-m.m[3][2]*m.m[0][1]))
					),
				////
		
				///2,
					(1 / det)*(
						 (m.m[3][1]*(m.m[0][2]*m.m[1][3]-m.m[0][3]*m.m[1][2]))
						+(m.m[3][2]*(m.m[0][3]*m.m[1][1]-m.m[0][1]*m.m[1][3]))
						+(m.m[3][3]*(m.m[0][1]*m.m[1][2]-m.m[0][2]*m.m[1][1]))
						),
				////

				///3
					-(1 / det)*(
						 (m.m[0][1]*(m.m[1][2]*m.m[2][3]-m.m[1][3]*m.m[2][2]))
						+(m.m[0][2]*(m.m[1][3]*m.m[2][1]-m.m[1][1]*m.m[2][3]))
						+(m.m[0][3]*(m.m[1][1]*m.m[2][2]-m.m[1][2]*m.m[2][1]))
						),
				////
				},
			////
			
			//1
				{
				///0
				-(1 / det)*(
						 (m.m[1][2]*(m.m[2][3]*m.m[3][0]-m.m[2][0]*m.m[3][3]))
						+(m.m[1][3]*(m.m[2][0]*m.m[3][2]-m.m[2][2]*m.m[3][0]))
						+(m.m[1][0]*(m.m[2][2]*m.m[3][3]-m.m[2][3]*m.m[3][2]))
						),
				////

				///1
				(1 / det)*(
						 (m.m[2][2]*(m.m[3][3]*m.m[0][0]-m.m[3][0]*m.m[0][3]))
						+(m.m[2][3]*(m.m[3][0]*m.m[0][2]-m.m[3][2]*m.m[0][0]))
						+(m.m[2][0]*(m.m[3][2]*m.m[0][3]-m.m[3][3]*m.m[0][2]))
						),
				////

				///2
				-(1 / det)*(
						 (m.m[3][2]*(m.m[0][3]*m.m[1][0]-m.m[0][0]*m.m[1][3]))
						+(m.m[3][3]*(m.m[0][0]*m.m[1][2]-m.m[0][2]*m.m[1][0]))
						+(m.m[3][0]*(m.m[0][2]*m.m[1][3]-m.m[0][3]*m.m[1][2]))
						),
				///3
				(1 / det)*(
						 (m.m[0][2]*(m.m[1][3]*m.m[2][0]-m.m[1][0]*m.m[2][3]))
						+(m.m[0][3]*(m.m[1][0]*m.m[2][2]-m.m[1][2]*m.m[2][0]))
						+(m.m[0][0]*(m.m[1][2]*m.m[2][3]-m.m[1][3]*m.m[2][2]))
						),
				////

				},
			////
			
			//2
				{
				///0
				(1 / det)*(
						 (m.m[1][3]*(m.m[2][0]*m.m[3][1]-m.m[2][1]*m.m[3][0]))
						+(m.m[1][0]*(m.m[2][1]*m.m[3][3]-m.m[2][3]*m.m[3][1]))
						+(m.m[1][1]*(m.m[2][3]*m.m[3][0]-m.m[2][0]*m.m[3][3]))
						),
				////

				///1
				-(1 / det)*(
						 (m.m[2][3]*(m.m[3][0]*m.m[0][1]-m.m[3][1]*m.m[0][0]))
						+(m.m[2][0]*(m.m[3][1]*m.m[0][3]-m.m[3][3]*m.m[0][1]))
						+(m.m[2][1]*(m.m[3][3]*m.m[0][0]-m.m[3][0]*m.m[0][3]))
						),
				////
				
				///2
				(1 / det)*(
						 (m.m[3][3]*(m.m[0][0]*m.m[1][1]-m.m[0][1]*m.m[1][0]))
						+(m.m[3][0]*(m.m[0][1]*m.m[1][3]-m.m[0][3]*m.m[1][1]))
						+(m.m[3][1]*(m.m[0][3]*m.m[1][0]-m.m[0][0]*m.m[1][3]))
						),
				////
			
				///3
				-(1 / det)*(
						 (m.m[0][3]*(m.m[1][0]*m.m[2][1]-m.m[1][1]*m.m[2][0]))
						+(m.m[0][0]*(m.m[1][1]*m.m[2][3]-m.m[1][3]*m.m[2][1]))
						+(m.m[0][1]*(m.m[1][3]*m.m[2][0]-m.m[1][0]*m.m[2][3]))
						),
				},
			////
			
			//3
				{
				///0
				-(1 / det)*(
						 (m.m[1][0]*(m.m[2][1]*m.m[3][2]-m.m[2][2]*m.m[3][1]))
						+(m.m[1][1]*(m.m[2][2]*m.m[3][0]-m.m[2][0]*m.m[3][2]))
						+(m.m[1][2]*(m.m[2][0]*m.m[3][1]-m.m[2][1]*m.m[3][0]))
						),
				///1
				(1 / det)*(
						 (m.m[2][0]*(m.m[3][1]*m.m[0][2]-m.m[3][2]*m.m[0][1]))
						+(m.m[2][1]*(m.m[3][2]*m.m[0][0]-m.m[3][0]*m.m[0][2]))
						+(m.m[2][2]*(m.m[3][0]*m.m[0][1]-m.m[3][1]*m.m[0][0]))
						),
				///2
				-(1 / det)*(
						 (m.m[3][0]*(m.m[0][1]*m.m[1][2]-m.m[0][2]*m.m[1][1]))
						+(m.m[3][1]*(m.m[0][2]*m.m[1][0]-m.m[0][0]*m.m[1][2]))
						+(m.m[3][2]*(m.m[0][0]*m.m[1][1]-m.m[0][1]*m.m[1][0]))
						),
				///3
				(1 / det)*(
						 (m.m[0][0]*(m.m[1][1]*m.m[2][2]-m.m[1][2]*m.m[2][1]))
						+(m.m[0][1]*(m.m[1][2]*m.m[2][0]-m.m[1][0]*m.m[2][2]))
						+(m.m[0][2]*(m.m[1][0]*m.m[2][1]-m.m[1][1]*m.m[2][0]))
						),
				}
			}
		};
		
		
	

		return result;
	}
	Matrix4x4 Transpose(const Matrix4x4& m)
	{
		
		return Matrix4x4(
		{
			
				{m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0]},
				{m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1]},
				{m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2]},
				{m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]}
		}
		);
	}
	Matrix4x4 Makeidetity4x4(){
		return Matrix4x4(
			{
				{1,0,0,0},
				{0,1,0,0},
				{0,0,1,0},
				{0,0,0,1}
			}
		
		);
	}

	//加算
	Vector3 Add(const Vector3& v1, const Vector3& v2){
		Vector3 result={
			v1.x + v2.x,
			v1.y + v2.y,
			v1.z + v2.z
		
		};
		return result;
	}
	//
	Vector3 Subtract(const Vector3& v1, const Vector3& v2){
		Vector3 result = {
			v1.x - v2.x,
			v1.y - v2.y,
			v1.z - v2.z
		};
		return result;
	}
	//
	Vector3 Multiply(float scalar, const Vector3& v){
		Vector3 result = {
			scalar * v.x,
			scalar * v.y,
			scalar * v.z
		};
		return result;
	}
	//
	float Dot(const Vector3& v1, const Vector3& v2){
		
		float result = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
		return result ;
	}
	//
	float Length(const Vector3& v){
		float length = v.x * v.x + v.y * v.y + v.z * v.z;
		if (length > 0.0f) {
			return sqrtf(length);
		}
		return length;
	}
	//
	Vector3 Normalize(const Vector3& v){
		float length = Length(v);
		if (length > 0.0f) {
			Vector3 result = {
				v.x / length,
				v.y / length,
				v.z / length
			};
			return result;
		}
		Vector3 result = { 0.0f, 0.0f, 0.0f };
		return result;
//
	}

    Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m)
    {
        Vector3 ret{
           ret.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0],
           ret.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1],
           ret.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2]
        };

        return ret;
    }
