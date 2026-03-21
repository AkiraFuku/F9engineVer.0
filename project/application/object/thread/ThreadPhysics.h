#pragma once

#include <vector>

#include "PhysicsNode.h"
#include "Vector4.h" // ※ Vector4.h から修正（環境に合わせて変更してください）

class ThreadPhysics {
public:
    /// <summary>
    /// 糸の初期化
    /// </summary>
    /// <param name="startPos">糸の始点</param>
    /// <param name="endPos">糸の終点</param>
    /// <param name="nodeCount">節の数</param>
    void Initialize(const Vector3& startPos, const Vector3& endPos, int nodeCount);

    /// <summary>
    /// 物理シミュレーションの更新
    /// </summary>
    void Update();

    /// <summary>
    /// 指定範囲内のノードに力を加える（Verlet積分における位置へのインパルス付与）
    /// </summary>
    /// <param name="pos">力を加える中心（プレイヤーや敵の座標）</param>
    /// <param name="radius">影響範囲の半径</param>
    /// <param name="force">加える力のベクトル（重み）</param>
    void ApplyForce(const Vector3& pos, float radius, const Vector3& force);

public:
    // ======================================
    // Getter
    // ======================================

    // ノード
    const std::vector<PhysicsNode>& GetNodes() const { return nodes_; }

public:
    // ======================================
    // Setter
    // ======================================

    // 重力
    void SetGravity(const Vector3& gravity) { gravity_ = gravity; }
    // 空気抵抗
    void SetDamping(float damping) { damping_ = damping; }
    // 制約を解く回数
    void SetIterations(int iterations) { iterations_ = iterations; }

private:
    // ======================================
    // ヘルパーメソッド
    // ======================================

    /// <summary>
    /// Verlet積分による各ノードの位置更新
    /// </summary>
    void Integrate();
    /// <summary>
    /// ノード間の距離制約の解決（長さの維持）
    /// </summary>
    void SolveConstraints();

private:
    std::vector<PhysicsNode> nodes_; // 糸を構成する節の配列
    float segmentLength_ = 0.0f;     // 節と節の間の理想的な距離

    // シミュレーションの調整パラメーター
    int iterations_ = 100;           // 制約を解く回数（多いほど硬く伸びない糸になる）
    float damping_ = 0.99f;          // 空気抵抗（1.0で抵抗なし）
    Vector3 gravity_ = {0.0f, -0.0005f, 0.0f}; // 重力
};