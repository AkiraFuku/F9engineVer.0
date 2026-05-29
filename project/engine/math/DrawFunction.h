#pragma once

#pragma once
#include "MathFunction.h"
#include "Vector4.h"
#include "Quaternion.h"
struct Sphere{
	Vector3 center;//中心
	float radius;
    Quaternion rotate={ 0.0f,0.0f,0.0f,1.0f }; // 回転を追加
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


Vector3 Project(const Vector3& v1,const Vector3& v2);
Vector3 ClosestPoint(const Vector3& point,const Segment& segment);
float DistanceSqPointToSegment(const Vector3& p, const Vector3& a, const Vector3& b);

bool IsCollision(const Sphere& si,const Sphere& s2);
bool IsCollision(const Sphere& sphere,const Plane& plane);
bool isCollision(const Segment& segment, const Plane& plane);
bool isCollision(const Line& line, const Plane& plane);
bool isCollision(const Ray& ray, const Plane& plane);
bool isCollision(const Ray& ray, const Triangle& triangle);
bool isCollision(const Triangle triangle , const  Segment& segment );
bool IsCollision(const AABB& a, const AABB& b);
bool IsCollision(const AABB& aabb,const Sphere& sphere);
bool IsCollision(const AABB& aabb,const Segment& segment);

