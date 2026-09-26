#pragma once
#include "Vector3.h"
#include "DrawFunction.h"
#include <vector>
#include <cstdint>
#include <cfloat>
#include <algorithm>

class Collider;
class GameObject;

// 衝突レイヤーマスク定義（Unityの LayerMask に準拠）
namespace CollisionLayer {
    constexpr uint32_t None       = 0;
    constexpr uint32_t Default    = 1 << 0;
    constexpr uint32_t Ground     = 1 << 1; // 地面・歩行可能斜面 (normal.y >= 0.7f)
    constexpr uint32_t Wall       = 1 << 2; // 垂直壁・急斜面 (|normal.y| < 0.7f)
    constexpr uint32_t OneWay     = 1 << 3; // すり抜け足場 (triangle.isOneway == true)
    constexpr uint32_t Player     = 1 << 4;
    constexpr uint32_t Enemy      = 1 << 5;
    constexpr uint32_t Projectile = 1 << 6;
    constexpr uint32_t All        = 0xFFFFFFFF;
}

// UnityスタイルのRaycastHit構造体
struct RaycastHit {
    bool hit = false;
    Vector3 point = { 0.0f, 0.0f, 0.0f };
    Vector3 normal = { 0.0f, 1.0f, 0.0f };
    float distance = FLT_MAX;
    Triangle triangle = {};
    Collider* collider = nullptr;
    GameObject* gameObject = nullptr;
    uint32_t layer = CollisionLayer::Default;
    RayTriangleCollisionResult faceResult = RayTriangleCollisionResult::NoCollision;
};

/// <summary>
/// Unityの Physics クラスに準拠した統合物理・衝突判定クラス
/// </summary>
class Physics {
public:
    // シーンのジオメトリ・コライダーリストの登録（静的API用）
    static void SetTriangles(const std::vector<Triangle>* triangles);
    static void SetColliders(const std::vector<Collider*>* colliders);
    static void Clear();

    // ─── 1. レイキャスト判定 (Raycast) ───────────────────────────
    /// <summary>
    /// 最も近い交差を判定（Unity: Physics.Raycast）
    /// </summary>
    static bool Raycast(const Ray& ray, RaycastHit* hitInfo, float maxDistance = 1000.0f, uint32_t layerMask = CollisionLayer::All);
    static bool Raycast(const Vector3& origin, const Vector3& direction, RaycastHit* hitInfo, float maxDistance = 1000.0f, uint32_t layerMask = CollisionLayer::All);
    static bool Raycast(const Ray& ray, const std::vector<Triangle>& triangles, RaycastHit* hitInfo, float maxDistance = 1000.0f, uint32_t layerMask = CollisionLayer::All);

    /// <summary>
    /// ヒットしたすべての交差を距離昇順で取得（Unity: Physics.RaycastAll）
    /// </summary>
    static std::vector<RaycastHit> RaycastAll(const Ray& ray, float maxDistance = 1000.0f, uint32_t layerMask = CollisionLayer::All);
    static std::vector<RaycastHit> RaycastAll(const Vector3& origin, const Vector3& direction, float maxDistance = 1000.0f, uint32_t layerMask = CollisionLayer::All);
    static std::vector<RaycastHit> RaycastAll(const Ray& ray, const std::vector<Triangle>& triangles, float maxDistance = 1000.0f, uint32_t layerMask = CollisionLayer::All);

    // ─── 2. 球体判定 (Sphere) ───────────────────────────────────
    /// <summary>
    /// 球体をレイ方向に掃引して衝突を判定（Unity: Physics.SphereCast）
    /// </summary>
    static bool SphereCast(const Vector3& origin, float radius, const Vector3& direction, RaycastHit* hitInfo, float maxDistance = 1000.0f, uint32_t layerMask = CollisionLayer::All);
    static bool SphereCast(const Vector3& origin, float radius, const Vector3& direction, const std::vector<Triangle>& triangles, RaycastHit* hitInfo, float maxDistance = 1000.0f, uint32_t layerMask = CollisionLayer::All);

    /// <summary>
    /// 球体内に対象が存在するか（Unity: Physics.CheckSphere）
    /// </summary>
    static bool CheckSphere(const Vector3& position, float radius, uint32_t layerMask = CollisionLayer::All);

    /// <summary>
    /// 球体内に存在するコライダー一覧を取得（Unity: Physics.OverlapSphere）
    /// </summary>
    static std::vector<Collider*> OverlapSphere(const Vector3& position, float radius, uint32_t layerMask = CollisionLayer::All);

    // ─── 3. ボックス・AABB判定 (Box / AABB) ──────────────────────
    /// <summary>
    /// ボックス・AABB領域内に対象が存在するか（Unity: Physics.CheckBox）
    /// </summary>
    static bool CheckBox(const Vector3& center, const Vector3& halfExtents, uint32_t layerMask = CollisionLayer::All);
    static bool CheckAABB(const AABB& aabb, uint32_t layerMask = CollisionLayer::All);

    /// <summary>
    /// ボックス・AABB領域内に存在するコライダー一覧を取得（Unity: Physics.OverlapBox）
    /// </summary>
    static std::vector<Collider*> OverlapBox(const Vector3& center, const Vector3& halfExtents, uint32_t layerMask = CollisionLayer::All);
    static std::vector<Collider*> OverlapAABB(const AABB& aabb, uint32_t layerMask = CollisionLayer::All);

    // ─── 4. 幾何形状交差判定ユーティリティ ─────────────────────────
    static bool Intersect(const Sphere& a, const Sphere& b);
    static bool Intersect(const AABB& a, const AABB& b);
    static bool Intersect(const Sphere& sphere, const AABB& aabb);
    static bool Intersect(const AABB& aabb, const Sphere& sphere);
    static bool Intersect(const Sphere& sphere, const Triangle& triangle, Vector3* outClosestPoint = nullptr);
    static bool Intersect(const AABB& aabb, const Triangle& triangle);
    static bool Intersect(const Ray& ray, const Sphere& sphere, float* outDistance = nullptr, Vector3* outHitPoint = nullptr);
    static bool Intersect(const Ray& ray, const AABB& aabb, float* outDistance = nullptr, Vector3* outHitPoint = nullptr);

    // 補助関数: 三角形の法線とプロパティからレイヤーを自動判定
    static uint32_t GetTriangleLayer(const Triangle& tri, const Vector3& normal);

    // 補助関数: 点から三角形への最近接点を計算
    static Vector3 ClosestPointOnTriangle(const Vector3& point, const Triangle& triangle);

private:
    static const std::vector<Triangle>* s_triangles_;
    static const std::vector<Collider*>* s_colliders_;
};
