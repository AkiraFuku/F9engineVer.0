#pragma once

#pragma once
#include "MathFunction.h"
#include "Vector4.h"
struct Sphere{
	Vector3 center;//中心
	float radius;
};
struct Line{
	Vector3 origin;//起点
	Vector3 diff;//方向

};
struct Ray{
	Vector3 origin;//起点
	Vector3 diff;//方向
};
struct  Segment{
	Vector3 origin;//起点
	Vector3 diff;//方向
};
struct Plane
{
	Vector3 normal;//法向量
	float distance;//距離
};
struct Triangle
{
	Vector3 vertices[3];//頂点1

};
struct AABB{
	Vector3 min;
	Vector3 max;
};




//void DrawSphere(
//	const Sphere& sphere,
//	const Matrix4x4& viewProjectionMatrix,
//	const Matrix4x4& viewportMatrix,
//	uint32_t color
//	
//);
//void DrawGrid(
//	const Matrix4x4& viewProjectionMatrix,
//	const Matrix4x4& viewportMatrix
//);
//void DrawPlane(
//	const Plane& plane,
//	const Matrix4x4& viewProjectionMatrix,
//	const Matrix4x4& viewportMatrix,
//	uint32_t color
//);
//void DrawSegment(
//	const Segment& segment,
//	const Matrix4x4& viewProjectionMatrix,
//	const Matrix4x4& viewportMatrix,
//	uint32_t color
//);
//void DrawTriangle(
//	const Triangle& triangle,
//	const Matrix4x4& viewProjectionMatrix,
//	const Matrix4x4& viewportMatrix,
//	uint32_t color
//);
//void DrawAABB(
//	const AABB& aabb,
//	const Matrix4x4& viewProjectionMatrix,
//	const Matrix4x4& viewportMatrix,
//	uint32_t color
//);
//void DrawBezier(
//	const Vector3& p0,
//	const Vector3& p1,
//	const Vector3& p2,
//	const Matrix4x4& viewProjectionMatrix,
//	const Matrix4x4& viewportMatrix,
//	uint32_t color
//);
//void DrawRail(
//	const Vector3& p1,
//	const Vector3& p2,
//	const Matrix4x4& viewProjectionMatrix,
//	const Matrix4x4& viewportMatrix,
//	uint32_t color
//);


Vector3 Project(const Vector3& v1,const Vector3& v2);
Vector3 ClosestPoint(const Vector3& point,const Segment& segment);
float DistanceSqPointToSegment(const Vector3& p, const Vector3& a, const Vector3& b);

bool IsCollision(const Sphere& si,const Sphere& s2);
bool IsCollision(const Sphere& sphere,const Plane& plane);
bool isCollision(const Segment& segment, const Plane& plane);
bool isCollision(const Line& line, const Plane& plane);
bool isCollision(const Ray& ray, const Plane& plane);
bool isCollision(const Triangle triangle , const  Segment& segment );
bool IsCollision(const AABB& a, const AABB& b);
bool IsCollision(const AABB& aabb,const Sphere& sphere);
bool IsCollision(const AABB& aabb,const Segment& segment);

