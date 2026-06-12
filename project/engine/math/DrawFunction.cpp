#include"DrawFunction.h"
#define NOMINMAX
#include<algorithm>
#include <functional>


/// <summary>
/// 正射影ベクトル
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns></returns>
Vector3 Project(const Vector3& v1, const Vector3& v2)
{
	float dot = Dot(v1, v2);
	float length = Length(v2);
	if (length == 0.0f) {return Vector3();} // v2がゼロベクトルの場合は投影できない

	
	return Multiply((dot / powf(length ,2)),v2);
}

Vector3 ClosestPoint(const Vector3& point, const Segment& segment)
{
	Vector3 project = Project(Subtract(point, segment.origin),segment.diff);
	return Add(segment.origin,project);
}

// 線分ABと点Pの最短距離の二乗を計算する関数
float DistanceSqPointToSegment(const Vector3& p, const Vector3& a, const Vector3& b) {
    Vector3 ab = b - a;
    Vector3 ap = p - a;

    // 内積を計算
    float d1 = ap.x * ab.x + ap.y * ab.y + ap.z * ab.z;
    if (d1 <= 0.0f) {
        // 点Pが点Aの外側にある場合、Aとの距離が最短
        return ap.x * ap.x + ap.y * ap.y + ap.z * ap.z;
    }

    float d2 = ab.x * ab.x + ab.y * ab.y + ab.z * ab.z;
    if (d2 <= d1) {
        // 点Pが点Bの外側にある場合、Bとの距離が最短
        Vector3 bp = p - b;
        return bp.x * bp.x + bp.y * bp.y + bp.z * bp.z;
    }

    // 点Pが線分ABの間にある場合、垂線の長さを計算
    float t = d1 / d2;
    Vector3 closestPoint = {
        a.x + t * ab.x,
        a.y + t * ab.y,
        a.z + t * ab.z
    };
    Vector3 diff = p - closestPoint;
    return diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
}

// sphereとsphereの衝突判定
bool IsCollision(const Sphere& s1, const Sphere& s2)
{
	float distance= Length(Subtract(s2.center,s1.center));
	if (distance<=s1.radius+s2.radius)
	{
		return true;
	}
	return false;
}
//SphereとPlaneの衝突判定
bool IsCollision(const Sphere& sphere, const Plane& plane)
{
	float distance = Dot(sphere.center, plane.normal) - plane.distance;
	if (fabs(distance) <= sphere.radius)
	{
		return true;
	}
	return false;
}

//SegmentとPlaneの衝突判定
bool isCollision(const Segment& segment, const Plane& plane)
{
	float dot = Dot(segment.diff, plane.normal);
	if (dot==0.0f)
	{
		return false;
	}
	float t = (plane.distance - Dot(segment.origin, plane.normal)) / dot;
	
	if (t<0.0f || t > 1.0f)
	{
		return false;
	}




	
	
	
	return true;

}
//LineとPlaneの衝突判定
bool isCollision(const Line& line, const Plane& plane)
{
	float dot = Dot(line.diff, plane.normal);
	if (dot==0.0f)
	{
		return false;
	}
	float t = (plane.distance - Dot(line.origin, plane.normal)) / dot;
	if (t<=0.0f || t > 1.0f)
	{
		return false;
	}
	
	
	return true;
}
//RayとPlaneの衝突判定
bool isCollision(const Ray& ray, const Plane& plane)
{
	float dot = Dot(ray.diff, plane.normal);
	if (dot==0.0f)
	{
		return false;
	}
	float t = (plane.distance - Dot(ray.origin, plane.normal)) / dot;
	if (t<=0.0f)
	{
		return false;
	}
	
	
	return true;
}
bool isCollision(const Ray& ray, const Triangle& triangle ){


    //三角形の頂点
    const Vector3& v0 = triangle.vertices[0];
    const Vector3& v1 = triangle.vertices[1];
    const Vector3& v2 = triangle.vertices[2];
    //三角形の辺を求める
    Vector3 v01 = Subtract(v1,v0);
    Vector3 v12 = Subtract(v2, v1);
    Vector3 v20 = Subtract(v0, v2);
    //三角形の法線ベクトルを求める
    Vector3 normal = Cross(v01, v12);
    //法線ベクトルを正規化
    normal = Normalize(normal);
    float d= Dot(v0,normal);
    float dot= Dot(ray.diff,normal);
    if (dot==0.0f)
    {
        return false;
    }
    float t = (d - Dot(ray.origin, normal)) / dot;
    if (t<=0.0f)
    {
        return false;
    }
    //レイの始点と終点を求める
    Vector3 p = Add(ray.origin, Multiply(t, ray.diff));
    //三角形の頂点からレイの始点までのベクトルを求める
    Vector3 v0p = Subtract(p, v0);
    Vector3 v1p = Subtract(p, v1);
    Vector3 v2p = Subtract(p, v2);
    //セグメントの始点と終点を求める
    Vector3 cross01=Cross(v01,v1p);
    Vector3 cross12=Cross(v12,v2p);
    Vector3 cross20=Cross(v20,v0p);
    if(	Dot(cross01, normal) >= 0.0f &&
       Dot(cross12, normal) >= 0.0f &&
        Dot(cross20, normal) >= 0.0f){
        return true;
    }
    return false;
}

bool isCollision(const Triangle triangle,const Segment& segment )
{

	
	 // 三角形の頂点
    const Vector3& v0 = triangle.vertices[0];
    const Vector3& v1 = triangle.vertices[1];
    const Vector3& v2 = triangle.vertices[2];
	 
//三角形の辺を求める
	Vector3 v01 = Subtract(v1,v0);
	Vector3 v12 = Subtract(v2, v1);
	Vector3 v21 = Subtract(v1, v2);
	Vector3 v20 = Subtract(v0, v2);
	
	Vector3 normal = Cross(v01, v12);

	//法線ベクトルを正規化
	normal = Normalize(normal);
// 線分の始点と終点
   
    Vector3 p0 = segment.origin;
    Vector3 p1 = Add(segment.origin, segment.diff);
  
	
	float d= Dot(v0,normal);
	float dot= Dot(segment.diff,normal);
	if (dot==0.0f)
	{
		return false;
	}
	float t = (d - Dot(p0, normal)) / dot;

	if (t<0.0f || t>1.0f)
	{
		return false;
	}
	//セグメントの始点と終点を求める
	Vector3 p = Add(p0, Multiply(t, segment.diff));
	

	//三角形の法線ベクトルを求める	
	//v01とv12の外積を求める
	
	

	//三角形の頂点からセグメントの始点までのベクトルを求める
	Vector3 v0p = Subtract(p, v0);
	Vector3 v1p = Subtract(p, v1);
	Vector3 v2p = Subtract(p, v2);

	

	//セグメントの始点と終点を求める

	Vector3 cross01=Cross(v01,v1p);
	Vector3 cross12=Cross(v12,v2p);
	Vector3 cross20=Cross(v20,v0p);

	if(	Dot(cross01, normal) >= 0.0f &&
	   Dot(cross12, normal) >= 0.0f &&
		Dot(cross20, normal) >= 0.0f){
		return true;
	}




	return false;
}

bool IsCollision(const AABB& a, const AABB& b)
{
	if ( (a.min.x<=b.max.x&&a.max.x>=b.min.x)&&
		 (a.min.y<=b.max.y&&a.max.y>=b.min.y)&&
		 (a.min.z<=b.max.z&&a.max.z>=b.min.z)){
		return true;
	}
	return false;
}

bool IsCollision(const AABB& aabb, const Sphere& sphere)
{
	Vector3 closestPoint = Vector3(
		std::clamp(sphere.center.x, aabb.min.x, aabb.max.x),
		std::clamp(sphere.center.y, aabb.min.y, aabb.max.y),
		std::clamp(sphere.center.z, aabb.min.z, aabb.max.z)
	);
	float distance = Length(Subtract(closestPoint, sphere.center));
	if (distance <= sphere.radius) {
		return true;
	}
	return false;
}

bool IsCollision(const AABB& aabb, const Segment& segment)
{

	Vector3 tVMin;
	Vector3 tVMax;
	tVMin = Division(Subtract(aabb.min, segment.origin),segment.diff);
	tVMax = Division(Subtract(aabb.max, segment.origin), segment.diff);
	Vector3 tNear={};
	Vector3 tFar;
	tNear.x = (std::min)(tVMin.x, tVMax.x);
	tNear.y = (std::min)(tVMin.y, tVMax.y);
	tNear.z = (std::min)(tVMin.z, tVMax.z);
	tFar.x = (std::max)(tVMin.x, tVMax.x);
	tFar.y = (std::max)(tVMin.y, tVMax.y);
	tFar.z = (std::max)(tVMin.z, tVMax.z);
	float tmin = (std::max)((std::max)(tNear.x, tNear.y), tNear.z);
	float tmax = (std::min)((std::min)(tFar.x, tFar.y), tFar.z);
	if (tmin<=tmax)
	{


		if (tmax>=0.0f && tmin<=1.0f)
		{
			//セグメントの始点と終点がAABBの内側にある場合は衝突している
			return true;
		}
		
	
	}


	return false;
}

bool CheckRayTriangle(const Ray& ray, const Triangle& triangle, float* outDistance, Vector3* outHitPoint) {
    // 三角形の頂点
    const Vector3& v0 = triangle.vertices[0];
    const Vector3& v1 = triangle.vertices[1];
    const Vector3& v2 = triangle.vertices[2];

    // 三角形の辺を求める
    Vector3 v01 = Subtract(v1, v0);
    Vector3 v12 = Subtract(v2, v1);
    Vector3 v20 = Subtract(v0, v2);

    // 三角形の法線ベクトルを求める
    Vector3 normal = Cross(v01, v12);
    float normalLen = Length(normal);
    if (normalLen == 0.0f) {
        return false;
    }
    // 法線ベクトルを正規化
    normal = Normalize(normal);

    float d = Dot(v0, normal);
    float dot = Dot(ray.diff, normal);
    if (dot == 0.0f) {
        return false;
    }

    float t = (d - Dot(ray.origin, normal)) / dot;
    if (t <= 0.0f) {
        return false;
    }

    // レイの射出点から交点までのベクトルを求める
    Vector3 p = Add(ray.origin, Multiply(t, ray.diff));

    // 三角形の各頂点から交点までのベクトルを求める
    Vector3 v0p = Subtract(p, v0);
    Vector3 v1p = Subtract(p, v1);
    Vector3 v2p = Subtract(p, v2);

    // 各辺の外積を求める
    Vector3 cross01 = Cross(v01, v1p);
    Vector3 cross12 = Cross(v12, v2p);
    Vector3 cross20 = Cross(v20, v0p);

    float d0 = Dot(cross01, normal);
    float d1 = Dot(cross12, normal);
    float d2 = Dot(cross20, normal);

    // すべての外積と法線の内積が同じ向き（すべて0以上、またはすべて0以下）なら交差している
    if ((d0 >= 0.0f && d1 >= 0.0f && d2 >= 0.0f) ||
        (d0 <= 0.0f && d1 <= 0.0f && d2 <= 0.0f)) {
        if (outDistance) {
            *outDistance = t;
        }
        if (outHitPoint) {
            *outHitPoint = p;
        }
        return true;
    }
    return false;
}

