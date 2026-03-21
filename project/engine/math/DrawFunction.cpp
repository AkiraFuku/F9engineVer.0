#include"DrawFunction.h"
#define NOMINMAX
#include<algorithm>
#include <functional>

//void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
//{
//	const uint32_t kSubdivision=24;//分割数
//	const float kLonEvery= 2.0f * PI / static_cast<float>(kSubdivision);
//	const float kLatEvery=PI / static_cast<float>(kSubdivision);
//	///緯度に分割
//	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex)
//	{
//		float lat = -PI/2.0f+kLatEvery*latIndex;//現在の緯度
//		float latD = PI / static_cast<float>(kSubdivision);//緯度の分割幅
//		//経度に分割
//		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; lonIndex++)
//		{
//			float lon=lonIndex*kLonEvery;//現在の経度
//			float lonD = PI * 2.0f / static_cast<float>(kSubdivision);//経度の分割幅
//			//world
//			Vector3 a,b,c;
//			a={
//				 sphere.center.x + sphere.radius * cosf(lat)*cosf(lon),
//				 sphere.center.y + sphere.radius *sinf(lat),
//				 sphere.center.z + sphere.radius *cosf(lat)*sinf(lon)
//			};
//			b={
//				 sphere.center.x + sphere.radius * cosf(lat+latD)*cosf(lon),
//				 sphere.center.y + sphere.radius *sin(lat+latD),
//				 sphere.center.z + sphere.radius * cosf(lat + latD) * sinf(lon)
//			};
//			c={ 
//				 sphere.center.x + sphere.radius * cosf(lat) * cosf(lon + lonD),
//				sphere.center.y + sphere.radius *sinf(lat),
//				sphere.center.z + sphere.radius *cosf(lat) * sinf(lon + lonD) };
//			//緯度線
//			Vector3 screen[3] = {};
//			for (uint32_t i = 0; i < 3; i++)
//			{
//				Vector3 nbc = Transform(i==0?a:i==1?b:c, viewProjectionMatrix);
//				screen[i] = Transform(nbc, viewportMatrix);
//			}
//			//描画
//		/*	Novice::DrawLine(static_cast<int>(screen[0].x), static_cast<int>(screen[0].y),
//								static_cast<int>(screen[1].x), static_cast<int>(screen[1].y), color);
//			Novice::DrawLine(static_cast<int>(screen[0].x), static_cast<int>(screen[0].y),
//				static_cast<int>(screen[2].x), static_cast<int>(screen[2].y), color);*/
//
//			
//			
//			
//		}
//
//	}
//
//}
//
//void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix)
//{
//	const float kGridHalfWidth=2.0f;//グリッドの半分
//	const uint32_t kSubdivision=10;//分割数
//	//一つ分の長さ
//	const float kGridEvery =(kGridHalfWidth*2.0f)/static_cast<float>(kSubdivision);
//
//	//奥から引いていく
//	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex)
//	{
//		Vector3 kGridLocalVertices[2]={};
//			kGridLocalVertices[0]={-kGridHalfWidth+static_cast<float>(xIndex)*static_cast<float>(kGridEvery),0.0f,kGridHalfWidth};
//			kGridLocalVertices[1]={-kGridHalfWidth+static_cast<float>(xIndex)*static_cast<float>(kGridEvery),0.0f,-kGridHalfWidth};
//
//		Vector3 screen[2]={};
//		for (uint32_t i = 0; i < 2; i++)
//		{
//
//
//			Vector3 nbc = Transform(kGridLocalVertices[i], viewProjectionMatrix);
//			screen[i] = Transform(nbc, viewportMatrix);
//		}
//
//		
//
//		if (xIndex==kSubdivision/2)
//		{
//			Novice::DrawLine(static_cast<int>(screen[0].x),static_cast<int>(screen[0].y),static_cast<int>(screen[1].x),static_cast<int>(screen[1].y),0x000000FF);
//
//		}else{
//			Novice::DrawLine(static_cast<int>(screen[0].x),static_cast<int>(screen[0].y),static_cast<int>(screen[1].x),static_cast<int>(screen[1].y),0xAAAAAAFF);
//		
//		}
//	}
//	for (uint32_t zIndex = 0; zIndex <= kSubdivision; zIndex++)
//	{
//		Vector3 kGridLocalVertices[2]={};
//			kGridLocalVertices[0]={kGridHalfWidth,0.0f,-kGridHalfWidth+static_cast<float>(zIndex)*static_cast<float>(kGridEvery)};
//			kGridLocalVertices[1]={-kGridHalfWidth,0.0f,-kGridHalfWidth+static_cast<float>(zIndex)*static_cast<float>(kGridEvery)};
//
//		Vector3 screen[2]={};
//		for (uint32_t i = 0; i < 2; i++)
//		{
//
//
//			Vector3 nbc = Transform(kGridLocalVertices[i], viewProjectionMatrix);
//			screen[i] = Transform(nbc, viewportMatrix);
//		}
//
//		
//		if (zIndex==kSubdivision/2)
//		{
//			Novice::DrawLine(static_cast<int>(screen[0].x),static_cast<int>(screen[0].y),static_cast<int>(screen[1].x),static_cast<int>(screen[1].y),0x000000FF);
//
//		}else{
//			Novice::DrawLine(static_cast<int>(screen[0].x),static_cast<int>(screen[0].y),static_cast<int>(screen[1].x),static_cast<int>(screen[1].y),0xAAAAAAFF);
//		
//		}
//		
//
//	}
//}
//
//void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
//{
//	Vector3 center = Multiply(plane.distance, plane.normal);
//	Vector3 perpendiculars[4];
//	perpendiculars[0] = Normalize(Perpendicular(plane.normal));
//	perpendiculars[1]={-perpendiculars[0].x ,-perpendiculars[0].y,-perpendiculars[0].z};
//	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]);
//	perpendiculars[3]={-perpendiculars[2].x ,-perpendiculars[2].y,-perpendiculars[2].z};
//
//	Vector3 points[4];
//	for (int i = 0; i < 4; ++i){
//		Vector3 extend =Multiply(2.0f,perpendiculars[i]);
//		Vector3 point = Add(center, extend);
//		points[i] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
//		Novice::ScreenPrintf(0,15*i,"point%d:x%0.0f:y%0.0f:z%0.0f",i,points[i].x,points[i].y,points[i].z);
//	}
//	Novice::DrawLine (
//		static_cast<int>(points[0].x),
//		static_cast<int>(points[0].y),
//		static_cast<int>(points[2].x),
//		static_cast<int>(points[2].y),
//			color);
//	Novice::DrawLine (
//		static_cast<int>(points[1].x),
//		static_cast<int>(points[1].y),
//		static_cast<int>(points[2].x),
//		static_cast<int>(points[2].y),
//			color);
//	Novice::DrawLine (
//		static_cast<int>(points[0].x),
//		static_cast<int>(points[0].y),
//		static_cast<int>(points[3].x),
//		static_cast<int>(points[3].y),
//			color);
//	Novice::DrawLine (
//		static_cast<int>(points[1].x),
//		static_cast<int>(points[1].y),
//		static_cast<int>(points[3].x),
//		static_cast<int>(points[3].y),
//			color);
//
//}
//
//void DrawSegment(const Segment& segment, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
//{
//	Vector3 point = Add(segment.origin, segment.diff);
//	Vector3 points[2];
//	for (int i = 0; i < 2; ++i){
//		
//
//		if (i==0)
//		{
//			points[i] = Transform(Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
//
//		} else
//		{
//			points[i] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
//
//		}
//	}
//	Novice::DrawLine(
//		static_cast<int>(points[0].x),
//		static_cast<int>(points[0].y),
//		static_cast<int>(points[1].x),
//		static_cast<int>(points[1].y),
//		color);
//}
//
//void DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
//{
//
//	Vector3 points[3];
//	for (int i = 0; i < 3; ++i){
//		points[i] = Transform(Transform(triangle.vertces[i], viewProjectionMatrix), viewportMatrix);
//
//		Novice::ScreenPrintf(0,15*i,"point%d:x%0.0f:y%0.0f:z%0.0f",i,points[i].x,points[i].y,points[i].z);
//	}
//	Novice::DrawLine(
//		static_cast<int>(points[0].x),
//		static_cast<int>(points[0].y),
//		static_cast<int>(points[1].x),
//		static_cast<int>(points[1].y),
//		color);
//	Novice::DrawLine(
//		static_cast<int>(points[1].x),
//		static_cast<int>(points[1].y),
//		static_cast<int>(points[2].x),
//		static_cast<int>(points[2].y),
//		color);
//	Novice::DrawLine(
//		static_cast<int>(points[2].x),
//		static_cast<int>(points[2].y),
//		static_cast<int>(points[0].x),
//		static_cast<int>(points[0].y),
//		color);
//
//}
//
//void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
//{
//	Vector3 points[8];
//	points[0] = Transform(Transform(aabb.min, viewProjectionMatrix), viewportMatrix);
//	points[1] = Transform(Transform(Vector3{ aabb.max.x, aabb.min.y, aabb.min.z }, viewProjectionMatrix), viewportMatrix);
//	points[2] = Transform(Transform(Vector3{ aabb.max.x, aabb.max.y, aabb.min.z }, viewProjectionMatrix), viewportMatrix);
//	points[3] = Transform(Transform(Vector3{ aabb.min.x, aabb.max.y, aabb.min.z }, viewProjectionMatrix), viewportMatrix);	
//	points[4] = Transform(Transform(Vector3{ aabb.min.x, aabb.min.y, aabb.max.z }, viewProjectionMatrix), viewportMatrix);
//	points[5] = Transform(Transform(Vector3{ aabb.max.x, aabb.min.y, aabb.max.z }, viewProjectionMatrix), viewportMatrix);
//	points[6] = Transform(Transform(aabb.max, viewProjectionMatrix), viewportMatrix);
//	points[7] = Transform(Transform(Vector3{ aabb.min.x, aabb.max.y, aabb.max.z }, viewProjectionMatrix), viewportMatrix);
//
//	Novice::DrawLine(
//		static_cast<int>(points[0].x),
//		static_cast<int>(points[0].y),
//		static_cast<int>(points[1].x),
//		static_cast<int>(points[1].y),
//		color);
//	Novice::DrawLine(
//		static_cast<int>(points[1].x),
//		static_cast<int>(points[1].y),
//		static_cast<int>(points[2].x),
//		static_cast<int>(points[2].y),
//		color);
//	Novice::DrawLine(
//		static_cast<int>(points[2].x),
//		static_cast<int>(points[2].y),
//		static_cast<int>(points[3].x),
//		static_cast<int>(points[3].y),
//		color);
//	Novice::DrawLine(
//		static_cast<int>(points[3].x),
//		static_cast<int>(points[3].y),
//		static_cast<int>(points[0].x),
//		static_cast<int>(points[0].y),
//		color);
//	Novice::DrawLine(
//		static_cast<int>(points[4].x),
//		static_cast<int>(points[4].y),
//		static_cast<int>(points[5].x),
//		static_cast<int>(points[5].y),
//		color);
//	Novice::DrawLine(
//		static_cast<int>(points[5].x),
//		static_cast<int>(points[5].y),
//		static_cast<int>(points[6].x),
//		static_cast<int>(points[6].y),
//		color);
//
//	Novice::DrawLine(
//		static_cast<int>(points[6].x),
//		static_cast<int>(points[6].y),
//		static_cast<int>(points[7].x),
//		static_cast<int>(points[7].y),
//		color);
//	Novice::DrawLine(
//		static_cast<int>(points[7].x),
//		static_cast<int>(points[7].y),
//		static_cast<int>(points[4].x),
//		static_cast<int>(points[4].y),
//		color);
//	Novice::DrawLine(
//		static_cast<int>(points[0].x),
//		static_cast<int>(points[0].y),
//		static_cast<int>(points[4].x),
//		static_cast<int>(points[4].y),
//		color);
//	Novice::DrawLine(
//		static_cast<int>(points[1].x),
//		static_cast<int>(points[1].y),
//		static_cast<int>(points[5].x),
//		static_cast<int>(points[5].y),
//		color);
//	Novice::DrawLine(
//		static_cast<int>(points[2].x),
//		static_cast<int>(points[2].y),
//		static_cast<int>(points[6].x),
//		static_cast<int>(points[6].y),
//		color);
//
//	Novice::DrawLine(
//		static_cast<int>(points[3].x),
//		static_cast<int>(points[3].y),
//		static_cast<int>(points[7].x),
//		static_cast<int>(points[7].y),
//		color);
//}
//
//void DrawBezier(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
//{
//	int numSegments = 32; // 分割数
//	for (int i = 0; i < numSegments; i++)
//	{
//		float t = static_cast<float>(i) / static_cast<float>(numSegments - 1);
//
//
//
//		Vector3 point1 = Bezier(p0, p1, p2, t);
//		Vector3 point2 = Bezier(p0, p1, p2, static_cast<float>(i+1) / static_cast<float>(numSegments - 1));
//		Vector3 point1Screen = Transform(Transform(point1, viewProjectionMatrix),viewportMatrix);
//		Vector3 point2Screen = Transform(Transform(point2, viewProjectionMatrix),viewportMatrix);
//
//		Novice ::DrawLine(
//			static_cast<int>(point1Screen.x),
//			static_cast<int>(point1Screen.y),
//			static_cast<int>(point2Screen.x),
//			static_cast<int>(point2Screen.y),
//			color);
//
//	}
//	DrawSphere({ p0, 0.05f }, viewProjectionMatrix, viewportMatrix, 0x0000FFFF);
//	DrawSphere({ p1, 0.05f }, viewProjectionMatrix, viewportMatrix, 0x0000FFFF);
//	DrawSphere({ p2, 0.05f }, viewProjectionMatrix, viewportMatrix, 0x0000FFFF);
//}
//
//void DrawRail(const Vector3& p1, const Vector3& p2, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
//{
//	Vector3 point1Screen = Transform(Transform(p1, viewProjectionMatrix),viewportMatrix);
//	Vector3 point2Screen = Transform(Transform(p2, viewProjectionMatrix),viewportMatrix);
//	Novice ::DrawLine(
//		static_cast<int>(point1Screen.x),
//		static_cast<int>(point1Screen.y),
//		static_cast<int>(point2Screen.x),
//		static_cast<int>(point2Screen.y),
//		color);
//}

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
